#undef FALSE
#define TRUE
#define USE_TIME_FORMAT

using System;
using System.Collections.Generic;
using System.Data;
using WindowsPerformanceReview;
using System.Windows.Forms.DataVisualization.Charting;
namespace WindowsPerformanceViewer {
    partial class BootTimesPlotForm {
        public static readonly String xName = "Index";
        private static readonly String[] colNames = {
            "Index",
            "BootTime",
            "MainPathBootTime",
            "BootPostBootTime",
        };
        /// <summary>
        /// Index of the colNames in the boot times list.  Must be synchronized
        /// with colNames.
        /// </summary>
        private static readonly int[] colDataIndices = {
            0,
            1,
            2,
            3,
        };
        private static readonly int nCols = colNames.Length;

        private List<String[]> bootTimes;

        public List<String[]> BootTimes {
            get { return bootTimes; }
            set { bootTimes = value; }
        }

        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// BootTimesPlotForm constructor.
        /// </summary>
        /// <param name="bootTimes">Value passed in by Form1 to save computational time.</param>
        public BootTimesPlotForm(List<String[]> bootTimes) {
            this.bootTimes = bootTimes;
            InitializeComponent();

            // Reassign the DataSource from what was done in InitializeComponent,
#if TRUE
            for (int i = 1; i < nCols; i++) {
                chart1.Series[i - 1].YValueMembers = colNames[i];
                chart1.Series[i - 1].XValueMember = colNames[0];
#if USE_TIME_FORMAT
                chart1.Series[i - 1].XValueType = ChartValueType.DateTime;
#endif
            }
            chart1.DataSource = getResultsTable();
            chart1.DataBind();
#else
            // reset chart control
            //chart1.ChartAreas.Clear();
            //chart1.Series.Clear();
            //chart1.ChartAreas.Add("Area1");

            // create dummy data source to bind
            DataTable dt = new DataTable();
            const string colnameNAME = "Name";
            const string colnameSCORE = "Score";
            dt.Columns.Add(colnameNAME, typeof(double));
            dt.Columns.Add(colnameSCORE, typeof(double));
            DataRow dr = dt.NewRow();
            dr[colnameNAME] = 1; ;
            dr[colnameSCORE] = 100;
            dt.Rows.Add(dr);
            dr = dt.NewRow();
            dr[colnameNAME] = 2;
            dr[colnameSCORE] = 20;
            dt.Rows.Add(dr);

            // connect data source to series
            //chart1.Series.Add("series0");
            chart1.Series[0].Name = "BootTime";
            chart1.Series[0].IsXValueIndexed = true;
            chart1.Series[0].YValueMembers = colnameSCORE;
            chart1.Series[0].XValueMember = colnameNAME;
            chart1.Series[0].ChartArea = "ChartArea1";
            chart1.Series[0].ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            // bind
            chart1.DataSource = dt;
            chart1.DataBind();
#endif
        }

        /// <summary>
        /// Builds a DataTable of the boot times data.
        /// </summary>
        public DataTable getResultsTable() {
            if (bootTimes == null) {
                // TODO
                return null;
            }
            // Statistics have been added to bootTimes by Form1
            // Set the count so the statistics are not included
            int count = bootTimes.Count - 4;
            if (count <= 0) {
                // TODO
                return null;
            }

            String[][] data = bootTimes.ToArray();
            int nItems = data[0].Length;

            // Create the output table.
            DataTable dt = new DataTable();

            // Add columns
            foreach (String colName in colNames) {
                dt.Columns.Add(colName, typeof(double));
            }

            // Add the values
            // This is the hard-coded index of BootTime  (Could have searched for it)
            double doubleVal;
            int index;
            DataRow dr;
            for (int j = 0; j < count; j++) {
                dr = dt.NewRow();
#if USE_TIME_FORMAT
                // The StartTime is at index 0.  Could have used colDataIndices[0];
                String startDate = data[j][0];
                DateTime dateTime = Utils.utcToLocalDateTime(startDate);
                dr[colNames[0]] = dateTime.ToOADate();
#else
                dr[colNames[0]] = j + 1;
#endif
                for (int i = 1; i < nCols; i++) {
                    index = colDataIndices[i];
                    try {
                        doubleVal = Convert.ToDouble(data[j][index]);
                    } catch (FormatException) {
                        doubleVal = Double.NaN;
                    } catch (OverflowException) {
                        doubleVal = Double.NaN;
                    }
                    // Don't convert to sec.  It has already been done.
                    dr[colNames[i]] = doubleVal;
                }
                dt.Rows.Add(dr);
            }
            return dt;
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
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
            System.Windows.Forms.DataVisualization.Charting.Series series2 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.Series series3 = new System.Windows.Forms.DataVisualization.Charting.Series();
            System.Windows.Forms.DataVisualization.Charting.Title title1 = new System.Windows.Forms.DataVisualization.Charting.Title();
            this.chart1 = new System.Windows.Forms.DataVisualization.Charting.Chart();
            ((System.ComponentModel.ISupportInitialize)(this.chart1)).BeginInit();
            this.SuspendLayout();
            // 
            // chart1
            // 
            this.chart1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            chartArea1.AxisX.Title = "Start Time";
            chartArea1.AxisY.Title = "Time (sec)";
            chartArea1.Name = "ChartArea1";
            this.chart1.ChartAreas.Add(chartArea1);
            legend1.Name = "Legend1";
            this.chart1.Legends.Add(legend1);
            this.chart1.Location = new System.Drawing.Point(2, 1);
            this.chart1.Name = "chart1";
            series1.ChartArea = "ChartArea1";
            series1.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            series1.Legend = "Legend1";
            series1.Name = "BootTime";
            series2.ChartArea = "ChartArea1";
            series2.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            series2.Legend = "Legend1";
            series2.Name = "MainPathBootTime";
            series3.ChartArea = "ChartArea1";
            series3.ChartType = System.Windows.Forms.DataVisualization.Charting.SeriesChartType.Line;
            series3.Legend = "Legend1";
            series3.Name = "BootPostBootTime";
            this.chart1.Series.Add(series1);
            this.chart1.Series.Add(series2);
            this.chart1.Series.Add(series3);
            this.chart1.Size = new System.Drawing.Size(734, 502);
            this.chart1.TabIndex = 0;
            this.chart1.Text = "chart1";
            title1.Font = new System.Drawing.Font("Microsoft Sans Serif", 12F, System.Drawing.FontStyle.Bold);
            title1.Name = "MainTitle";
            title1.Text = "Boot Times";
            this.chart1.Titles.Add(title1);
            // 
            // BootTimesPlotForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(738, 501);
            this.Controls.Add(this.chart1);
            this.Name = "BootTimesPlotForm";
            this.Text = "Boot Times";
            ((System.ComponentModel.ISupportInitialize)(this.chart1)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.DataVisualization.Charting.Chart chart1;
    }
}