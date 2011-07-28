﻿#undef FALSE
#define TRUE
#define USE_TIME_FORMAT

using System;
using System.Collections.Generic;
using System.Data;
using WindowsPerformanceReview;
using System.Windows.Forms.DataVisualization.Charting;
namespace WindowsPerformanceViewer {
    partial class PlotForm {
        public static readonly String xName = "Index";
        private static readonly String[] colNames = {
            "Index",
            "BootTime",
            "MainPathBootTime",
            "BootPostBootTime",
            "BootExplorerInitTime",
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
            18,
        };
        private static readonly int nCols = colNames.Length;

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

            // Reassign the DataSource from what was done in InitializeComponent
            // The series index is 1 less than the column index since
            // the x vales are in column index 0
            chart1.Series.Clear();
            for (int i = 1; i < nCols; i++) {
                chart1.Series.Add(colNames[i]);
                chart1.Series[i - 1].YValueMembers = colNames[i];
                chart1.Series[i - 1].XValueMember = colNames[0];
                chart1.Series[i - 1].XValueType = ChartValueType.DateTime;
            }
            chart1.DataSource = getResultsTable();
            chart1.DataBind();
        }

        /// <summary>
        /// Builds a DataTable of the boot times data.
        /// </summary>
        public DataTable getResultsTable() {
            double[][] data = model.Data;
            if (data == null) {
                return null;
            }
            int count = data.Length;
            if (count <= 0) {
                // TODO
                return null;
            }
            int nItems = data[0].Length;

            // Create the output table.
            DataTable dt = new DataTable();

            // Add columns
            foreach (String colName in colNames) {
                dt.Columns.Add(colName, typeof(double));
            }

            // Add the values
            double doubleVal;
            int index;
            DataRow dr;
            for (int j = 0; j < count; j++) {
                dr = dt.NewRow();
                // The StartTime is at index 0.  Could have used colDataIndices[0];
                dr[colNames[0]] = data[j][0];
                for (int i = 1; i < nCols; i++) {
                    index = colDataIndices[i];
                    doubleVal = data[j][index];
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
            chartArea1.AxisY.Title = "Parameter Value";
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
            title1.Text = "Boot Parameters";
            this.chart1.Titles.Add(title1);
            // 
            // PlotForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(738, 501);
            this.Controls.Add(this.chart1);
            this.Name = "PlotForm";
            this.Text = "Boot Parameters";
            ((System.ComponentModel.ISupportInitialize)(this.chart1)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.DataVisualization.Charting.Chart chart1;
    }
}