using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

using TerraView.com.terraserver_usa;
using KEUtilities;

namespace TerraView
{
	/// <summary>
	/// Summary description for PlaceDlg.
	/// </summary>
    public class PlaceDlg : System.Windows.Forms.Form {
        private Int32 maxItems=25;
        bool valid=false;
        private double centerLon=0.0, centerLat=0.0;
        PlaceFacts[] placeFacts=null;
        MainForm mainForm=null;
        TerraService ts=null;

        private System.Windows.Forms.Button searchBTN;
        private System.Windows.Forms.Button okBTN;
        private System.Windows.Forms.Button cancelBTN;
        private System.Windows.Forms.TextBox placeNameTB;
        private System.Windows.Forms.ListBox placeChoiceLB;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.Container components = null;

        public PlaceDlg(MainForm mainFormIn, TerraService tsIn) {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            //
            // TODO: Add any constructor code after InitializeComponent call
            //
            mainForm=mainFormIn;
            ts=tsIn;
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
            this.searchBTN = new System.Windows.Forms.Button();
            this.okBTN = new System.Windows.Forms.Button();
            this.cancelBTN = new System.Windows.Forms.Button();
            this.placeNameTB = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.placeChoiceLB = new System.Windows.Forms.ListBox();
            this.SuspendLayout();
            // 
            // searchBTN
            // 
            this.searchBTN.Location = new System.Drawing.Point(21, 144);
            this.searchBTN.Name = "searchBTN";
            this.searchBTN.TabIndex = 4;
            this.searchBTN.Text = "Search";
            this.searchBTN.Click += new System.EventHandler(this.OnSearchClicked);
            // 
            // okBTN
            // 
            this.okBTN.Location = new System.Drawing.Point(109, 144);
            this.okBTN.Name = "okBTN";
            this.okBTN.TabIndex = 5;
            this.okBTN.Text = "OK";
            this.okBTN.Click += new System.EventHandler(this.OnOkClicked);
            // 
            // cancelBTN
            // 
            this.cancelBTN.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelBTN.Location = new System.Drawing.Point(197, 144);
            this.cancelBTN.Name = "cancelBTN";
            this.cancelBTN.TabIndex = 6;
            this.cancelBTN.Text = "Cancel";
            // 
            // placeNameTB
            // 
            this.placeNameTB.Location = new System.Drawing.Point(8, 24);
            this.placeNameTB.Name = "placeNameTB";
            this.placeNameTB.Size = new System.Drawing.Size(272, 20);
            this.placeNameTB.TabIndex = 1;
            this.placeNameTB.Text = "";
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(8, 48);
            this.label2.Name = "label2";
            this.label2.TabIndex = 2;
            this.label2.Text = "Search results:";
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(8, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(208, 23);
            this.label1.TabIndex = 0;
            this.label1.Text = "Enter the name of a place to search for:";
            // 
            // placeChoiceLB
            // 
            this.placeChoiceLB.Location = new System.Drawing.Point(8, 64);
            this.placeChoiceLB.Name = "placeChoiceLB";
            this.placeChoiceLB.Size = new System.Drawing.Size(272, 69);
            this.placeChoiceLB.TabIndex = 3;
            // 
            // PlaceDlg
            // 
            this.AcceptButton = this.searchBTN;
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.CancelButton = this.cancelBTN;
            this.ClientSize = new System.Drawing.Size(292, 181);
            this.Controls.AddRange(new System.Windows.Forms.Control[] {
                                                                          this.placeChoiceLB,
                                                                          this.placeNameTB,
                                                                          this.label2,
                                                                          this.cancelBTN,
                                                                          this.okBTN,
                                                                          this.searchBTN,
                                                                          this.label1});
            this.Name = "PlaceDlg";
            this.Text = "Place to Map";
            this.ResumeLayout(false);

        }
		#endregion

        public bool Valid {
            get { return valid; }
            set { valid=value; }
        }

        public double CenterLon {
            get { return centerLon; }
            set { centerLon=value; }
        }
        
        public double CenterLat {
            get { return centerLat; }
            set { centerLat=value; }
        }
 
        private void OnSearchClicked(object sender, System.EventArgs e) {
            // Set wait cursor
            Cursor.Current=Cursors.WaitCursor;

            // Search
            try {
                placeFacts=ts.GetPlaceList(placeNameTB.Text,maxItems,true);
            } catch {
                ErrMsg.Show("Could not search");
                Cursor.Current=Cursors.Default;
                return;
            }
            
            // Fill the list box
            placeChoiceLB.Items.Clear();
            for(int i=0; i < placeFacts.Length; i++) {
                placeChoiceLB.Items.Add(placeFacts[i].Place.City + ", " +
                    placeFacts[i].Place.State + ", " +
                    placeFacts[i].Place.Country);
                // Set the first item to be selected
                if(i == 0) placeChoiceLB.SelectedIndex=i;
            }

            // Set the accept button to the Ok button
            this.AcceptButton=okBTN;

            Cursor.Current=Cursors.Default;
        }

        private void OnOkClicked(object sender, System.EventArgs e) {
            valid=false;
            if(placeChoiceLB.Items.Count == 0 ||
                placeChoiceLB.SelectedIndex < 0) {
                ErrMsg.Show("Nothing selected");
                return;
            }
            int sel=placeChoiceLB.SelectedIndex;
            centerLon=placeFacts[sel].Center.Lon;
            centerLat=placeFacts[sel].Center.Lat;
            valid=true;
            DialogResult=DialogResult.OK;
            Close();
        }
    }
}
