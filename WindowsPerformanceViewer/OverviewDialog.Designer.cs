namespace WindowsPerformanceViewer {
    partial class OverviewDialog {
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
            this.overviewTextBox = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // overviewTextBox
            // 
            this.overviewTextBox.Dock = System.Windows.Forms.DockStyle.Fill;
            this.overviewTextBox.Location = new System.Drawing.Point(0, 0);
            this.overviewTextBox.Multiline = true;
            this.overviewTextBox.Name = "overviewTextBox";
            this.overviewTextBox.ReadOnly = true;
            this.overviewTextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.overviewTextBox.Size = new System.Drawing.Size(488, 361);
            this.overviewTextBox.TabIndex = 0;
            // 
            // OverviewDialog
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(8F, 16F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(488, 361);
            this.Controls.Add(this.overviewTextBox);
            this.Name = "OverviewDialog";
            this.Text = "Overview";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox overviewTextBox;
    }
}