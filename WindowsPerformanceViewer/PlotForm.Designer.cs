#undef FALSE
#define TRUE
#define USE_TIME_FORMAT

using System;
using System.Collections.Generic;
using System.Data;
using System.Windows.Forms.DataVisualization.Charting;

namespace WindowsPerformanceViewer {
    partial class PlotForm {
        public static readonly String xName = "Index";

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
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

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
            listNames = new String[BootDataModel.N_DATA_SETS];
            for (int i = 0; i < BootDataModel.N_DATA_SETS; i++) {
                listNames[i] = BootDataModel.BOOT_DATA_INFOS[i].Name;
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
        /// Set the data in the chart.
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
            DataType type = DataType.STRING;
            for (int i = 0; i < nCols; i++) {
                index = model.getIndexFor(names[i]);
                if (index < 0) {
                    Utils.errMsg("Name not found: " + names[i]);
                    break;
                }
                indices[i] = index;
                if (i == 0) {
                    // x axis
                    // Get the type to be used for the series
                    type = model.BootDataInfos[index].Type;
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
                    // The StartTime is at index 0.
                    dr[names[0]] = data[j][indices[0]];
                    for (int i = 1; i < nCols; i++) {
                        index = indices[i];
                        doubleVal = data[j][index];
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

#if FALSE
            String info = "";
            info += "Interval=" + chart1.ChartAreas[0].AxisX.Interval + Utils.LF;
            info += "Minimum=" + chart1.ChartAreas[0].AxisX.Minimum + Utils.LF;
            info += "Maximum=" + chart1.ChartAreas[0].AxisX.Maximum + Utils.LF;
            Utils.infoMsg(info);
#endif
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

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be
        /// disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing) {
            if (disposing && (components != null)) {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent() {
            System.Windows.Forms.DataVisualization.Charting.ChartArea chartArea1 = new System.Windows.Forms.DataVisualization.Charting.ChartArea();
            System.Windows.Forms.DataVisualization.Charting.Legend legend1 = new System.Windows.Forms.DataVisualization.Charting.Legend();
            System.Windows.Forms.DataVisualization.Charting.Series series1 = new System.Windows.Forms.DataVisualization.Charting.Series();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.listBox1 = new System.Windows.Forms.ListBox();
            this.panel1 = new System.Windows.Forms.Panel();
            this.printButton = new System.Windows.Forms.Button();
            this.resetButton = new System.Windows.Forms.Button();
            this.setYButton = new System.Windows.Forms.Button();
            this.setXButton = new System.Windows.Forms.Button();
            this.chart1 = new System.Windows.Forms.DataVisualization.Charting.Chart();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.panel1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.chart1)).BeginInit();
            this.SuspendLayout();
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.listBox1);
            this.splitContainer1.Panel1.Controls.Add(this.panel1);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.chart1);
            this.splitContainer1.Size = new System.Drawing.Size(989, 551);
            this.splitContainer1.SplitterDistance = 225;
            this.splitContainer1.TabIndex = 0;
            // 
            // listBox1
            // 
            this.listBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.listBox1.FormattingEnabled = true;
            this.listBox1.ItemHeight = 16;
            this.listBox1.Location = new System.Drawing.Point(0, 0);
            this.listBox1.Name = "listBox1";
            this.listBox1.SelectionMode = System.Windows.Forms.SelectionMode.MultiExtended;
            this.listBox1.Size = new System.Drawing.Size(225, 468);
            this.listBox1.TabIndex = 0;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.printButton);
            this.panel1.Controls.Add(this.resetButton);
            this.panel1.Controls.Add(this.setYButton);
            this.panel1.Controls.Add(this.setXButton);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Bottom;
            this.panel1.Location = new System.Drawing.Point(0, 469);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(225, 82);
            this.panel1.TabIndex = 1;
            // 
            // printButton
            // 
            this.printButton.Location = new System.Drawing.Point(107, 40);
            this.printButton.Name = "printButton";
            this.printButton.Size = new System.Drawing.Size(75, 23);
            this.printButton.TabIndex = 4;
            this.printButton.Text = "Print";
            this.printButton.UseVisualStyleBackColor = true;
            this.printButton.Click += new System.EventHandler(this.printButton_Click);
            // 
            // resetButton
            // 
            this.resetButton.Location = new System.Drawing.Point(12, 40);
            this.resetButton.Name = "resetButton";
            this.resetButton.Size = new System.Drawing.Size(75, 23);
            this.resetButton.TabIndex = 3;
            this.resetButton.Text = "Reset";
            this.resetButton.UseVisualStyleBackColor = true;
            this.resetButton.Click += new System.EventHandler(this.resetButton_Click);
            // 
            // setYButton
            // 
            this.setYButton.Location = new System.Drawing.Point(12, 11);
            this.setYButton.Name = "setYButton";
            this.setYButton.Size = new System.Drawing.Size(75, 23);
            this.setYButton.TabIndex = 2;
            this.setYButton.Text = "Set Y";
            this.setYButton.UseVisualStyleBackColor = true;
            this.setYButton.Click += new System.EventHandler(this.setYButton_Click);
            // 
            // setXButton
            // 
            this.setXButton.Location = new System.Drawing.Point(107, 11);
            this.setXButton.Name = "setXButton";
            this.setXButton.Size = new System.Drawing.Size(75, 23);
            this.setXButton.TabIndex = 0;
            this.setXButton.Text = "Set X";
            this.setXButton.UseVisualStyleBackColor = true;
            this.setXButton.Click += new System.EventHandler(this.setXButton_Click);
            // 
            // chart1
            // 
            chartArea1.Name = "ChartArea1";
            this.chart1.ChartAreas.Add(chartArea1);
            this.chart1.Dock = System.Windows.Forms.DockStyle.Fill;
            legend1.Name = "Legend1";
            this.chart1.Legends.Add(legend1);
            this.chart1.Location = new System.Drawing.Point(0, 0);
            this.chart1.Name = "chart1";
            series1.ChartArea = "ChartArea1";
            series1.Legend = "Legend1";
            series1.Name = "Series1";
            this.chart1.Series.Add(series1);
            this.chart1.Size = new System.Drawing.Size(760, 551);
            this.chart1.TabIndex = 0;
            this.chart1.Text = "chart1";
            // 
            // PlotForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(989, 551);
            this.Controls.Add(this.splitContainer1);
            this.Name = "PlotForm";
            this.Text = "Boot Parameters";
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.chart1)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.ListBox listBox1;
        private Chart chart1;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Button printButton;
        private System.Windows.Forms.Button resetButton;
        private System.Windows.Forms.Button setYButton;
        private System.Windows.Forms.Button setXButton;

    }
}