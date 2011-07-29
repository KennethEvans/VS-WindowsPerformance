using System;
using System.Collections.Generic;
using System.Data;
using System.Windows.Forms;

namespace WindowsPerformanceViewer {
    partial class Form1 {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        private static readonly String[] colHeadings = {
            "(Times are in sec)",
            "BootTime",
            "MainPathBootTime",
            "BootPostBootTime",
        };
        private static readonly int nCols = colHeadings.Length;
        private static readonly String[] rowHeadings = {
            "Last",
            "Mean",
            "Standard Deviation",
            "Min",
            "Max",
        };
        private static readonly int nRows = rowHeadings.Length;
        public static readonly double MS2SEC = .001;

        List<String[]> bootTimes;

        public List<String[]> BootTimes {
            get { return bootTimes; }
            set { bootTimes = value; }
        }

        public Form1() {
            InitializeComponent();
            // Create the DataSource
            DataTable table = getResultsTable();
            if (table == null) {
                MessageBox.Show("Unable to get data.  You may have to run as administrator.",
                    "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            dataGridView1.DataSource = getResultsTable();
            dataGridView1.Columns[0].DefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleLeft;
            //dataGridView1.Columns[1].DefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleCenter;
            //dataGridView1.Columns[2].DefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleCenter;
            //dataGridView1.Columns[3].DefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleCenter;
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
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle1 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle2 = new System.Windows.Forms.DataGridViewCellStyle();
            this.dataGridView1 = new System.Windows.Forms.DataGridView();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createCSVFileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.plotExtendedToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).BeginInit();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // dataGridView1
            // 
            this.dataGridView1.AllowUserToAddRows = false;
            this.dataGridView1.AllowUserToDeleteRows = false;
            this.dataGridView1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.dataGridView1.AutoSizeColumnsMode = System.Windows.Forms.DataGridViewAutoSizeColumnsMode.AllCells;
            this.dataGridView1.AutoSizeRowsMode = System.Windows.Forms.DataGridViewAutoSizeRowsMode.AllCells;
            this.dataGridView1.BackgroundColor = System.Drawing.Color.White;
            dataGridViewCellStyle1.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleCenter;
            dataGridViewCellStyle1.BackColor = System.Drawing.SystemColors.Control;
            dataGridViewCellStyle1.Font = new System.Drawing.Font("Microsoft Sans Serif", 7.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle1.ForeColor = System.Drawing.SystemColors.WindowText;
            dataGridViewCellStyle1.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle1.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle1.WrapMode = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridView1.ColumnHeadersDefaultCellStyle = dataGridViewCellStyle1;
            this.dataGridView1.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            dataGridViewCellStyle2.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleCenter;
            dataGridViewCellStyle2.BackColor = System.Drawing.SystemColors.Window;
            dataGridViewCellStyle2.Font = new System.Drawing.Font("Microsoft Sans Serif", 7.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle2.ForeColor = System.Drawing.SystemColors.ControlText;
            dataGridViewCellStyle2.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle2.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle2.WrapMode = System.Windows.Forms.DataGridViewTriState.False;
            this.dataGridView1.DefaultCellStyle = dataGridViewCellStyle2;
            this.dataGridView1.Location = new System.Drawing.Point(0, 36);
            this.dataGridView1.Name = "dataGridView1";
            this.dataGridView1.ReadOnly = true;
            this.dataGridView1.RowTemplate.Height = 24;
            this.dataGridView1.Size = new System.Drawing.Size(638, 213);
            this.dataGridView1.TabIndex = 0;
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.RenderMode = System.Windows.Forms.ToolStripRenderMode.System;
            this.menuStrip1.Size = new System.Drawing.Size(637, 36);
            this.menuStrip1.TabIndex = 1;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.createCSVFileToolStripMenuItem,
            this.plotExtendedToolStripMenuItem,
            this.toolStripMenuItem1,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(54, 32);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // createCSVFileToolStripMenuItem
            // 
            this.createCSVFileToolStripMenuItem.Name = "createCSVFileToolStripMenuItem";
            this.createCSVFileToolStripMenuItem.Size = new System.Drawing.Size(215, 32);
            this.createCSVFileToolStripMenuItem.Text = "Create CSV File";
            this.createCSVFileToolStripMenuItem.Click += new System.EventHandler(this.createCSVFileToolStripMenuItem_Click);
            // 
            // plotExtendedToolStripMenuItem
            // 
            this.plotExtendedToolStripMenuItem.Name = "plotExtendedToolStripMenuItem";
            this.plotExtendedToolStripMenuItem.Size = new System.Drawing.Size(215, 32);
            this.plotExtendedToolStripMenuItem.Text = "Plots...";
            this.plotExtendedToolStripMenuItem.Click += new System.EventHandler(this.plotToolStripMenuItem_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(212, 6);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(215, 32);
            this.exitToolStripMenuItem.Text = "Exit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(65, 32);
            this.helpToolStripMenuItem.Text = "Help";
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(139, 32);
            this.aboutToolStripMenuItem.Text = "About";
            this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoSize = true;
            this.BackColor = System.Drawing.SystemColors.Control;
            this.ClientSize = new System.Drawing.Size(637, 249);
            this.Controls.Add(this.dataGridView1);
            this.Controls.Add(this.menuStrip1);
            this.Name = "Form1";
            this.Text = "Boot Performance";
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).EndInit();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        /// <summary>
        /// Builds a DataTable of the boot times data.
        /// </summary>
        public DataTable getResultsTable() {
            bootTimes = DiagnosticsUtils.getBootTimes();
            DiagnosticsUtils.setStatistics(bootTimes);
            if (bootTimes == null) {
                // TODO
                return null;
            }
            int count = bootTimes.Count;
            // Do the last nLast data points
            int nLast = 5;
            // The last item before the statistics is count -5
            int lastPos = count - 4 - nLast;
            if (lastPos <= 0) {
                // TODO
                return null;
            }

            String[][] data = bootTimes.ToArray();
            int nItems = data[0].Length;

            // Convert all the times to sec
            double doubleVal;
            for (int j = 0; j < count; j++) {
                for (int i = 1; i < nCols; i++) {
                    try {
                        doubleVal = Convert.ToDouble(data[j][i]);
                    } catch (FormatException) {
                        doubleVal = Double.NaN;
                    } catch (OverflowException) {
                        doubleVal = Double.NaN;
                    }
                    doubleVal *= MS2SEC;
                    data[j][i] = String.Format("{0:0.0}", doubleVal);
                }
            }

            // Create the output table.
            DataTable dt = new DataTable();

            // Add the columns
            foreach (string heading in colHeadings) {
                dt.Columns.Add(heading, typeof(String));
            }

            // Add the rows
            for (int i = lastPos; i < count; i++) {
                dt.Rows.Add(data[i][0], data[i][1], data[i][2], data[i][3]);
            }
            return dt;
        }

        private System.Windows.Forms.DataGridView dataGridView1;
        private MenuStrip menuStrip1;
        private ToolStripMenuItem fileToolStripMenuItem;
        private ToolStripMenuItem createCSVFileToolStripMenuItem;
        private ToolStripSeparator toolStripMenuItem1;
        private ToolStripMenuItem exitToolStripMenuItem;
        private ToolStripMenuItem plotExtendedToolStripMenuItem;
        private ToolStripMenuItem helpToolStripMenuItem;
        private ToolStripMenuItem aboutToolStripMenuItem;
    }
}

