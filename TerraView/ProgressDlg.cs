using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace TerraView
{
	/// <summary>
	/// Summary description for ProgressDlg.
	/// </summary>
	public class ProgressDlg : System.Windows.Forms.Form
	{
        private String itemsName=String.Empty;
        private bool cancelled;

        private System.Windows.Forms.ProgressBar progressBar;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button cancelBTN;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public ProgressDlg()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
			}
			base.Dispose( disposing );
		}

		#region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(ProgressDlg));
            this.progressBar = new System.Windows.Forms.ProgressBar();
            this.label1 = new System.Windows.Forms.Label();
            this.cancelBTN = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // progressBar
            // 
            this.progressBar.Location = new System.Drawing.Point(8, 32);
            this.progressBar.Name = "progressBar";
            this.progressBar.Size = new System.Drawing.Size(282, 23);
            this.progressBar.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(8, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(281, 16);
            this.label1.TabIndex = 2;
            // 
            // cancelBTN
            // 
            this.cancelBTN.Location = new System.Drawing.Point(108, 62);
            this.cancelBTN.Name = "cancelBTN";
            this.cancelBTN.TabIndex = 3;
            this.cancelBTN.Text = "Cancel";
            this.cancelBTN.Click += new System.EventHandler(this.OnCancel);
            // 
            // ProgressDlg
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(298, 92);
            this.ControlBox = false;
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                          this.cancelBTN,
                                                                          this.progressBar,
                                                                          this.label1});
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ProgressDlg";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Progress";
            this.ResumeLayout(false);

        }
		#endregion
   
        public int Maximum {
            get { return progressBar.Maximum; }
            set { progressBar.Maximum=value; }
        }

        public int Minimum {
            get { return progressBar.Minimum; }
            set { progressBar.Minimum=value; }
        }

        public int Value {
            get { return progressBar.Value; }
            set { progressBar.Value=value; }
        }

        public int Step {
            get { return progressBar.Step; }
            set { progressBar.Step=value; }
        }

        public String ItemsName {
            get { return itemsName; }
            set { itemsName=value; }
        }

        public bool Cancelled {
            get { return cancelled; }
            set { cancelled=value; }
        }

        public void PerformStep() {
            label1.Text="Processed " + progressBar.Value + " of " +
                progressBar.Maximum + " " + itemsName;
            progressBar.PerformStep();
        }

        public void Reset() {
            label1.Text="Processed 0 of " +
                progressBar.Maximum + " " + itemsName;
            cancelled=false;
        }

        private void OnCancel(object sender, System.EventArgs e) {
            cancelled=true;
        }
    }
}

