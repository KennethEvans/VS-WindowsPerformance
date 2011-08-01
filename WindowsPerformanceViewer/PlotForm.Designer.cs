#define USE_TIME_FORMAT

using System.Windows.Forms.DataVisualization.Charting;

namespace WindowsPerformanceViewer {
    partial class PlotForm {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

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