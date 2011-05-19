using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace KEUtilities {
    /// <summary>
    /// Summary description for ExDlg.
    /// </summary>
    public class ExDlg : System.Windows.Forms.Form {
        private System.Windows.Forms.TextBox textBox1;
        private System.Windows.Forms.TextBox textBox2;
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.Container components = null;

        public ExDlg(String s, Exception e) {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            //
            // TODO: Add any constructor code after InitializeComponent call
            //
            this.Message=s;
#if false
            String temp=
                "W1=" + textBox1.Width + "," + "NA" +
                "\nH1=" + textBox1.Height + "," + textBox1.PreferredHeight +
                "\nW2=" + textBox2.Width + "," + "NA" +
                "\nH2=" + textBox2.Height + "," + textBox2.PreferredHeight +
                "\n";
            this.ExceptionMessage=temp + e.ToString();
#else
            this.ExceptionMessage=e.ToString();
#endif
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose( bool disposing ) {
            if( disposing ) {
                if(components != null) {
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
        private void InitializeComponent() {
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(ExDlg));
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.textBox2 = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // textBox1
            // 
            this.textBox1.BackColor = System.Drawing.SystemColors.Control;
            this.textBox1.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.textBox1.ForeColor = System.Drawing.SystemColors.ControlText;
            this.textBox1.Location = new System.Drawing.Point(8, 8);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.ReadOnly = true;
            this.textBox1.Size = new System.Drawing.Size(432, 88);
            this.textBox1.TabIndex = 1;
            this.textBox1.Text = "Message";
            // 
            // textBox2
            // 
            this.textBox2.BackColor = System.Drawing.SystemColors.Control;
            this.textBox2.Location = new System.Drawing.Point(8, 104);
            this.textBox2.Multiline = true;
            this.textBox2.Name = "textBox2";
            this.textBox2.ReadOnly = true;
            this.textBox2.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBox2.Size = new System.Drawing.Size(432, 128);
            this.textBox2.TabIndex = 0;
            this.textBox2.Text = "ExceptionMessage";
            this.textBox2.WordWrap = false;
            // 
            // ExDlg
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(448, 245);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                          this.textBox2,
                                                                          this.textBox1});
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "ExDlg";
            this.Text = "Exception";
            this.Resize += new System.EventHandler(this.OnResize);
            this.ResumeLayout(false);

        }
		#endregion

        public String Message {
            get { return String.Join("\n",textBox1.Lines); }
            set { textBox1.Lines=value.Split('\n'); }
        }

        public String ExceptionMessage {
            get { return String.Join("\n",textBox2.Lines); }
            set { textBox2.Lines=value.Split('\n'); }
        }
        
        private void textBox1_TextChanged(object sender, System.EventArgs e) {
        
        }

        private void OnResize(object sender, System.EventArgs e) {
#if true
            int margin=8;
            textBox1.Width=this.ClientSize.Width-2*margin;
            textBox2.Width=this.ClientSize.Width-2*margin;
            textBox2.Height=this.ClientSize.Height-textBox1.Height-3*margin;
#endif
        }
    }
}

