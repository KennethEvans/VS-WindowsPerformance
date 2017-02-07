using System.Windows.Forms;

namespace WindowsPerformanceViewer {
    partial class Form1 {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

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
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle11 = new System.Windows.Forms.DataGridViewCellStyle();
            System.Windows.Forms.DataGridViewCellStyle dataGridViewCellStyle12 = new System.Windows.Forms.DataGridViewCellStyle();
            this.dataGridView1 = new System.Windows.Forms.DataGridView();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.createCSVFileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.plotExtendedToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.eventDescriptionToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.bootEventsMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.shutdownEventsMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.standbyEventsMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.monitoringEventsMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.otherEventsMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.runEventViewerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.overviewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripSeparator3 = new System.Windows.Forms.ToolStripSeparator();
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
            dataGridViewCellStyle11.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleCenter;
            dataGridViewCellStyle11.BackColor = System.Drawing.SystemColors.Control;
            dataGridViewCellStyle11.Font = new System.Drawing.Font("Microsoft Sans Serif", 7.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle11.ForeColor = System.Drawing.SystemColors.WindowText;
            dataGridViewCellStyle11.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle11.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle11.WrapMode = System.Windows.Forms.DataGridViewTriState.True;
            this.dataGridView1.ColumnHeadersDefaultCellStyle = dataGridViewCellStyle11;
            this.dataGridView1.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            dataGridViewCellStyle12.Alignment = System.Windows.Forms.DataGridViewContentAlignment.MiddleCenter;
            dataGridViewCellStyle12.BackColor = System.Drawing.SystemColors.Window;
            dataGridViewCellStyle12.Font = new System.Drawing.Font("Microsoft Sans Serif", 7.8F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            dataGridViewCellStyle12.ForeColor = System.Drawing.SystemColors.ControlText;
            dataGridViewCellStyle12.SelectionBackColor = System.Drawing.SystemColors.Highlight;
            dataGridViewCellStyle12.SelectionForeColor = System.Drawing.SystemColors.HighlightText;
            dataGridViewCellStyle12.WrapMode = System.Windows.Forms.DataGridViewTriState.False;
            this.dataGridView1.DefaultCellStyle = dataGridViewCellStyle12;
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
            this.toolStripSeparator2,
            this.eventDescriptionToolStripMenuItem,
            this.toolStripSeparator3,
            this.bootEventsMenuItem,
            this.shutdownEventsMenuItem,
            this.standbyEventsMenuItem,
            this.monitoringEventsMenuItem,
            this.otherEventsMenuItem,
            this.toolStripSeparator1,
            this.runEventViewerToolStripMenuItem,
            this.toolStripMenuItem1,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(54, 32);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // createCSVFileToolStripMenuItem
            // 
            this.createCSVFileToolStripMenuItem.Name = "createCSVFileToolStripMenuItem";
            this.createCSVFileToolStripMenuItem.Size = new System.Drawing.Size(278, 32);
            this.createCSVFileToolStripMenuItem.Text = "Create Boot CSV File";
            this.createCSVFileToolStripMenuItem.Click += new System.EventHandler(this.createCSVFileMenuItem_Click);
            // 
            // plotExtendedToolStripMenuItem
            // 
            this.plotExtendedToolStripMenuItem.Name = "plotExtendedToolStripMenuItem";
            this.plotExtendedToolStripMenuItem.Size = new System.Drawing.Size(278, 32);
            this.plotExtendedToolStripMenuItem.Text = "Boot Plots...";
            this.plotExtendedToolStripMenuItem.Click += new System.EventHandler(this.plotMenuItem_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(275, 6);
            // 
            // eventDescriptionToolStripMenuItem
            // 
            this.eventDescriptionToolStripMenuItem.Name = "eventDescriptionToolStripMenuItem";
            this.eventDescriptionToolStripMenuItem.Size = new System.Drawing.Size(278, 32);
            this.eventDescriptionToolStripMenuItem.Text = "Provider Information...";
            this.eventDescriptionToolStripMenuItem.Click += new System.EventHandler(this.eventDescriptionMenuItem_Click);
            // 
            // bootEventsMenuItem
            // 
            this.bootEventsMenuItem.Name = "bootEventsMenuItem";
            this.bootEventsMenuItem.Size = new System.Drawing.Size(278, 32);
            this.bootEventsMenuItem.Text = "Boot Events...";
            this.bootEventsMenuItem.Click += new System.EventHandler(this.bootEventsMenuItem_Click);
            // 
            // shutdownEventsMenuItem
            // 
            this.shutdownEventsMenuItem.Name = "shutdownEventsMenuItem";
            this.shutdownEventsMenuItem.Size = new System.Drawing.Size(278, 32);
            this.shutdownEventsMenuItem.Text = "Shutdown Events...";
            this.shutdownEventsMenuItem.Click += new System.EventHandler(this.shutdownEventsMenuItem_Click);
            // 
            // standbyEventsMenuItem
            // 
            this.standbyEventsMenuItem.Name = "standbyEventsMenuItem";
            this.standbyEventsMenuItem.Size = new System.Drawing.Size(278, 32);
            this.standbyEventsMenuItem.Text = "Standby Events...";
            this.standbyEventsMenuItem.Click += new System.EventHandler(this.standbyEventsMenuItem_Click);
            // 
            // monitoringEventsMenuItem
            // 
            this.monitoringEventsMenuItem.Name = "monitoringEventsMenuItem";
            this.monitoringEventsMenuItem.Size = new System.Drawing.Size(278, 32);
            this.monitoringEventsMenuItem.Text = "Monitoring Events...";
            this.monitoringEventsMenuItem.Click += new System.EventHandler(this.monitoringEventsMenuItem_Click);
            // 
            // otherEventsMenuItem
            // 
            this.otherEventsMenuItem.Name = "otherEventsMenuItem";
            this.otherEventsMenuItem.Size = new System.Drawing.Size(278, 32);
            this.otherEventsMenuItem.Text = "Other Events...";
            this.otherEventsMenuItem.Click += new System.EventHandler(this.otherEventsMenuItem_Click);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(275, 6);
            // 
            // runEventViewerToolStripMenuItem
            // 
            this.runEventViewerToolStripMenuItem.Name = "runEventViewerToolStripMenuItem";
            this.runEventViewerToolStripMenuItem.Size = new System.Drawing.Size(278, 32);
            this.runEventViewerToolStripMenuItem.Text = "Run Event Viewer";
            this.runEventViewerToolStripMenuItem.Click += new System.EventHandler(this.runEventViewerMenuItem_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(275, 6);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(278, 32);
            this.exitToolStripMenuItem.Text = "Exit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.overviewToolStripMenuItem,
            this.aboutToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(65, 32);
            this.helpToolStripMenuItem.Text = "Help";
            // 
            // overviewToolStripMenuItem
            // 
            this.overviewToolStripMenuItem.Name = "overviewToolStripMenuItem";
            this.overviewToolStripMenuItem.Size = new System.Drawing.Size(165, 32);
            this.overviewToolStripMenuItem.Text = "Overview";
            this.overviewToolStripMenuItem.Click += new System.EventHandler(this.overviewMenuItem_Click);
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(165, 32);
            this.aboutToolStripMenuItem.Text = "About";
            this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutMenuItem_Click);
            // 
            // toolStripSeparator3
            // 
            this.toolStripSeparator3.Name = "toolStripSeparator3";
            this.toolStripSeparator3.Size = new System.Drawing.Size(275, 6);
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
            this.Text = "Windows Performance";
            ((System.ComponentModel.ISupportInitialize)(this.dataGridView1)).EndInit();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.DataGridView dataGridView1;
        private MenuStrip menuStrip1;
        private ToolStripMenuItem fileToolStripMenuItem;
        private ToolStripMenuItem createCSVFileToolStripMenuItem;
        private ToolStripSeparator toolStripMenuItem1;
        private ToolStripMenuItem exitToolStripMenuItem;
        private ToolStripMenuItem plotExtendedToolStripMenuItem;
        private ToolStripMenuItem helpToolStripMenuItem;
        private ToolStripMenuItem aboutToolStripMenuItem;
        private ToolStripMenuItem overviewToolStripMenuItem;
        private ToolStripMenuItem eventDescriptionToolStripMenuItem;
        private ToolStripMenuItem runEventViewerToolStripMenuItem;
        private ToolStripSeparator toolStripSeparator1;
        private ToolStripMenuItem bootEventsMenuItem;
        private ToolStripMenuItem shutdownEventsMenuItem;
        private ToolStripMenuItem standbyEventsMenuItem;
        private ToolStripSeparator toolStripSeparator2;
        private ToolStripMenuItem monitoringEventsMenuItem;
        private ToolStripMenuItem otherEventsMenuItem;
        private ToolStripSeparator toolStripSeparator3;
    }
}

