using System;
using System.Collections.Generic;
using System.Data;
using System.Windows.Forms;
using System.Windows.Forms.DataVisualization.Charting;

namespace WindowsPerformanceViewer {
    public partial class PlotForm : Form {
        public static readonly String INDEX_NAME = "EventIndex";

        /// <summary>
        /// Roundoff value
        /// </summary>
        private static readonly double ROFF6 = .000001;

        /// <summary>
        /// The default names to plot. 0 corresponds to x.
        /// </summary>
        private static readonly String[] colNames0 = {
            "BootStartTime",
            "BootTime",
            "MainPathBootTime",
            "BootPostBootTime",
        };

        /// <summary>
        /// The currently used names to plot. 0 corresponds to x.
        /// </summary>
        private String[] colNames;

        private String[] listNames;

        private BootDataModel model = null;

        /// <summary>
        /// PlotForm constructor.
        /// </summary>
        /// <param name="bootTimes">Value passed in by Form1 to save computational time.</param>
        public PlotForm() {
            model = new BootDataModel();
            Boolean res = model.getData();
            if (!res) {
                Utils.errMsg("Cannot get boot data");
                return;
            }
            InitializeComponent();

            // Populate the list box
            listNames = new String[BootDataModel.N_DATA_SETS + 1];
            listNames[0] = INDEX_NAME;
            for (int i = 0; i < BootDataModel.N_DATA_SETS; i++) {
                listNames[i + 1] = BootDataModel.BOOT_DATA_INFOS[i].Name;
            }
            listBox1.DataSource = listNames;

            // Initialize colNames
            int len = colNames0.Length;
            colNames = new String[len];
            for (int i = 0; i < len; i++) {
                colNames[i] = (String)colNames0[i].Clone();
            }

            // Set chart parameters
            chart1.Titles.Add("Boot Parameters");
            chart1.Titles[0].Font =
                new System.Drawing.Font("Microsoft Sans Serif", 12F,
                    System.Drawing.FontStyle.Bold);
            chart1.ChartAreas[0].AxisY.Title = "Parameter";

            // Set the chart data to the defaults
            setChartValues(colNames);
        }

        /// <summary>
        /// Set the data in the chart.  Note: The chart data are in a small
        /// DataTable with an added index column.  The model data represents
        /// all the data and is much larger.  This method generates the data
        /// for the specified plot values from the model.
        /// </summary>
        /// <param name="names">Array of names of plot.  The first element
        /// is for the x axis.  The others are for the series.</param>
        public void setChartValues(String[] names) {
            chart1.Series.Clear();
            int nCols = names.Length;
            if (nCols < 2) {
                Utils.errMsg("Nothing specified to plot");
                return;
            }

            // Set the series
            int index;
            int[] indices = new int[nCols];
            DataType type = DataType.DOUBLE;
            for (int i = 0; i < nCols; i++) {
                index = getIndex(names[i]);
                if (index < 0) {
                    Utils.errMsg("Name not found: " + names[i]);
                    break;
                }
                indices[i] = index;
                if (i == 0) {
                    // x axis
                    // Get the type to be used for the x axis for the series
                    type = getType(index);
                    // This is not a series
                    continue;
                }
                chart1.Series.Add(names[i]);
                chart1.Series[i - 1].YValueMembers = names[i];
                chart1.Series[i - 1].XValueMember = names[0];
                chart1.Series[i - 1].ChartType = SeriesChartType.Line;
                if (type == DataType.DATE) {
                    chart1.Series[i - 1].XValueType = ChartValueType.DateTime;
                    chart1.ChartAreas[0].AxisX.IntervalType = DateTimeIntervalType.Auto;
                } else {
                    chart1.Series[i - 1].XValueType = ChartValueType.Auto;
                    // Seems to necessary to get it to reset from dates
                    chart1.ChartAreas[0].AxisX.IntervalType = DateTimeIntervalType.Number;
                }
            }
            chart1.ChartAreas[0].AxisX.Title = names[0];
            // Seems to be necessary to get labels on the x axis
            chart1.ChartAreas[0].AxisX.IntervalAutoMode = IntervalAutoMode.VariableCount;
            chart1.ChartAreas[0].AxisX.IsMarginVisible = true;
            // Rounds the tick values
            // Seems to eliminate the axis not ending on a grid line problem
            // Doesn't help if x axis is dates
            chart1.ChartAreas[0].AxisX.RoundAxisValues();

            // Make the DataTable
            DataTable dt = null;
            try {
                double[][] data = model.Data;
                if (data == null) {
                    Utils.errMsg("The plot data is null");
                    return;
                }
                int count = data.Length;
                if (count <= 0) {
                    Utils.errMsg("The plot data is empty");
                    return;
                }
                int nItems = data[0].Length;

                // Create the output table.
                dt = new DataTable();

                // Add columns
                foreach (String colName in names) {
                    dt.Columns.Add(colName, typeof(double));
                }

                // Add the values
                double doubleVal;
                DataRow dr;
                for (int j = 0; j < count; j++) {
                    dr = dt.NewRow();
                    // The first column is the x axis
                    dr[names[0]] = getDataPoint(j, indices[0]);
                    for (int i = 1; i < nCols; i++) {
                        index = indices[i];
                        doubleVal = getDataPoint(j, index);
                        dr[names[i]] = doubleVal;
                    }
                    dt.Rows.Add(dr);
                }
            } catch (Exception ex) {
                Utils.excMsg("Error creating data table", ex);
                return;
            }

            // Set the data source and bind
            if (dt != null) {
                colNames = names;
                chart1.DataSource = dt;
                chart1.DataBind();
            }

#if false
            String info = "";
            info += "Interval=" + chart1.ChartAreas[0].AxisX.Interval + Utils.LF;
            info += "Minimum=" + chart1.ChartAreas[0].AxisX.Minimum + Utils.LF;
            info += "Maximum=" + chart1.ChartAreas[0].AxisX.Maximum + Utils.LF;
            Utils.infoMsg(info);
#endif
        }

        /// <summary>
        /// Gets the index of a data set name in the DataTable taking into
        /// account the index column added at the beginning.  Error handling
        /// should be done by the caller.
        /// </summary>
        /// <param name="name">The data set name.</param>
        /// <returns></returns>
        private int getIndex(String name) {
            if (name.Equals(INDEX_NAME)) {
                return 0;
            }
            // Look for a model index
            int modelIndex = model.getIndexFor(name);
            if (modelIndex < 0) {
                return modelIndex;
            }
            return modelIndex + 1;
        }

        /// <summary>
        /// Gets the DataType of a data set name in the DataTable taking into
        /// account the index column added at the beginning.  The caller must
        /// pass a valid index.  The validity is not checked.
        /// </summary>
        /// <param name="index">The extended, not model, index.</param>
        /// <returns></returns>
        private DataType getType(int index) {
            if (index == 0) {
                return DataType.DOUBLE;
            }
            return model.BootDataInfos[index - 1].Type;
        }

        private double getDataPoint(int j, int index) {
            if (index == 0) {
                // Add roundoff so plot axis will start at 0
                return j + ROFF6;
            }
            return model.Data[j][index - 1];
        }

        /// <summary>
        /// Gets a list of the currently selected names in the ListBox
        /// </summary>
        /// <returns></returns>
        public String[] getListBoxSelectedNames() {
            if (listBox1 == null) {
                return null;
            }
            List<String> list = new List<String>();
            for (int i = 0; i < listBox1.Items.Count; i++) {
                if (listBox1.GetSelected(i)) {
                    list.Add(listBox1.Items[i].ToString());
                }
            }
            return list.ToArray();
        }

        /////////////////////////////////////////////////////////////////////
        // Event Handlers 
        /////////////////////////////////////////////////////////////////////

        private void setYButton_Click(object sender, EventArgs e) {
            String[] selected = getListBoxSelectedNames();
            int nSelected = selected.Length;
            if (selected.Length <= 0) {
                Utils.errMsg("No items selected");
                return;
            }
            String[] newNames = new String[nSelected + 1];
            newNames[0] = colNames[0];
            for (int i = 0; i < nSelected; i++) {
                newNames[i + 1] = selected[i];
            }
            setChartValues(newNames);
        }

        private void setXButton_Click(object sender, EventArgs e) {
            String[] selected = getListBoxSelectedNames();
            int nSelected = selected.Length;
            if (selected.Length <= 0) {
                Utils.errMsg("No items selected");
                return;
            }
            if (selected.Length > 1) {
                Utils.errMsg("More than 1 items selected");
                return;
            }
            // Change the 0 element of the current names
            colNames[0] = (String)selected[0].Clone();
            setChartValues(colNames);
        }

        private void resetButton_Click(object sender, EventArgs e) {
            int len = colNames0.Length;
            String[] newNames = new String[len];
            for (int i = 0; i < len; i++) {
                newNames[i] = (String)colNames0[i].Clone();
            }
            setChartValues(newNames);
        }

        private void printButton_Click(object sender, EventArgs e) {
            PrintingManager printMgr = chart1.Printing;
            printMgr.Print(true);
        }
    }
}
