using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

using System.Drawing.Drawing2D;
using TerraView.com.msrmaps;
using KEUtilities;

namespace TerraView
{
	/// <summary>
	/// Summary description for ControlPanel.
	/// </summary>
	public class ControlPanel : System.Windows.Forms.Form
	{
        private MainForm mainForm=null;
        private Region north,west,east,south,northEast,southEast,
            southWest,northWest,zoomIn,zoomOut;
        private bool alwaysOnTop=true;

        private System.Windows.Forms.Button controlPanelOKBtn;
        private System.Windows.Forms.Button controlPanelCancelBtn;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.ComboBox typeCB;
        private System.Windows.Forms.ComboBox scaleCB;
        private System.Windows.Forms.TextBox imageWidthTB;
        private System.Windows.Forms.TextBox centerLonTB;
        private System.Windows.Forms.TextBox centerLatTB;
        private System.Windows.Forms.TextBox imageHeightTB;
        private System.Windows.Forms.PictureBox navMapPB;
        private System.Windows.Forms.CheckBox alwaysOnTopCHK;
        private System.Windows.Forms.NumericUpDown panFactorUpDown;
        private System.Windows.Forms.Button controlPanelApplyButton;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public ControlPanel(MainForm mainFormIn)
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
            mainForm=mainFormIn;

            // Make the hit regions
            MakeHitRegions();
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
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(ControlPanel));
            this.controlPanelOKBtn = new System.Windows.Forms.Button();
            this.controlPanelCancelBtn = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.typeCB = new System.Windows.Forms.ComboBox();
            this.label2 = new System.Windows.Forms.Label();
            this.scaleCB = new System.Windows.Forms.ComboBox();
            this.label3 = new System.Windows.Forms.Label();
            this.label4 = new System.Windows.Forms.Label();
            this.centerLonTB = new System.Windows.Forms.TextBox();
            this.centerLatTB = new System.Windows.Forms.TextBox();
            this.imageWidthTB = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.imageHeightTB = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.navMapPB = new System.Windows.Forms.PictureBox();
            this.alwaysOnTopCHK = new System.Windows.Forms.CheckBox();
            this.panFactorUpDown = new System.Windows.Forms.NumericUpDown();
            this.label7 = new System.Windows.Forms.Label();
            this.controlPanelApplyButton = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.panFactorUpDown)).BeginInit();
            this.SuspendLayout();
            // 
            // controlPanelOKBtn
            // 
            this.controlPanelOKBtn.DialogResult = System.Windows.Forms.DialogResult.OK;
            this.controlPanelOKBtn.Location = new System.Drawing.Point(15, 264);
            this.controlPanelOKBtn.Name = "controlPanelOKBtn";
            this.controlPanelOKBtn.Size = new System.Drawing.Size(56, 23);
            this.controlPanelOKBtn.TabIndex = 12;
            this.controlPanelOKBtn.Text = "Go";
            this.controlPanelOKBtn.Click += new System.EventHandler(this.OnOkClick);
            // 
            // controlPanelCancelBtn
            // 
            this.controlPanelCancelBtn.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.controlPanelCancelBtn.Location = new System.Drawing.Point(155, 264);
            this.controlPanelCancelBtn.Name = "controlPanelCancelBtn";
            this.controlPanelCancelBtn.Size = new System.Drawing.Size(56, 23);
            this.controlPanelCancelBtn.TabIndex = 13;
            this.controlPanelCancelBtn.Text = "Cancel";
            this.controlPanelCancelBtn.Click += new System.EventHandler(this.OnCancelClick);
            // 
            // label1
            // 
            this.label1.Location = new System.Drawing.Point(8, 7);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(80, 23);
            this.label1.TabIndex = 0;
            this.label1.Text = "MapType:";
            this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // typeCB
            // 
            this.typeCB.Location = new System.Drawing.Point(96, 8);
            this.typeCB.Name = "typeCB";
            this.typeCB.Size = new System.Drawing.Size(121, 21);
            this.typeCB.TabIndex = 1;
            // 
            // label2
            // 
            this.label2.Location = new System.Drawing.Point(16, 31);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(80, 23);
            this.label2.TabIndex = 2;
            this.label2.Text = "Scale:";
            this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // scaleCB
            // 
            this.scaleCB.Location = new System.Drawing.Point(96, 32);
            this.scaleCB.Name = "scaleCB";
            this.scaleCB.Size = new System.Drawing.Size(121, 21);
            this.scaleCB.TabIndex = 3;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(8, 55);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(80, 23);
            this.label3.TabIndex = 4;
            this.label3.Text = "Longitude:";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(8, 79);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(80, 23);
            this.label4.TabIndex = 6;
            this.label4.Text = "Latitude:";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // centerLonTB
            // 
            this.centerLonTB.Location = new System.Drawing.Point(96, 56);
            this.centerLonTB.Name = "centerLonTB";
            this.centerLonTB.TabIndex = 5;
            this.centerLonTB.Text = "";
            // 
            // centerLatTB
            // 
            this.centerLatTB.Location = new System.Drawing.Point(96, 80);
            this.centerLatTB.Name = "centerLatTB";
            this.centerLatTB.TabIndex = 7;
            this.centerLatTB.Text = "";
            // 
            // imageWidthTB
            // 
            this.imageWidthTB.Location = new System.Drawing.Point(96, 104);
            this.imageWidthTB.Name = "imageWidthTB";
            this.imageWidthTB.TabIndex = 9;
            this.imageWidthTB.Text = "";
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(8, 103);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(80, 23);
            this.label5.TabIndex = 8;
            this.label5.Text = "Image Width:";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // imageHeightTB
            // 
            this.imageHeightTB.Location = new System.Drawing.Point(96, 128);
            this.imageHeightTB.Name = "imageHeightTB";
            this.imageHeightTB.TabIndex = 11;
            this.imageHeightTB.Text = "";
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(8, 127);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(80, 23);
            this.label6.TabIndex = 10;
            this.label6.Text = "Image Height:";
            this.label6.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // navMapPB
            // 
            this.navMapPB.Image = ((System.Drawing.Image)(resources.GetObject("navMapPB.Image")));
            this.navMapPB.Location = new System.Drawing.Point(8, 160);
            this.navMapPB.Name = "navMapPB";
            this.navMapPB.Size = new System.Drawing.Size(100, 99);
            this.navMapPB.TabIndex = 15;
            this.navMapPB.TabStop = false;
            this.navMapPB.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OnNavMouseDown);
            // 
            // alwaysOnTopCHK
            // 
            this.alwaysOnTopCHK.Location = new System.Drawing.Point(120, 216);
            this.alwaysOnTopCHK.Name = "alwaysOnTopCHK";
            this.alwaysOnTopCHK.TabIndex = 16;
            this.alwaysOnTopCHK.Text = "Always on Top";
            this.alwaysOnTopCHK.CheckedChanged += new System.EventHandler(this.OnAlwaysOnTopChanged);
            // 
            // panFactorUpDown
            // 
            this.panFactorUpDown.DecimalPlaces = 1;
            this.panFactorUpDown.Location = new System.Drawing.Point(160, 176);
            this.panFactorUpDown.Name = "panFactorUpDown";
            this.panFactorUpDown.Size = new System.Drawing.Size(48, 20);
            this.panFactorUpDown.TabIndex = 15;
            this.panFactorUpDown.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.panFactorUpDown.ValueChanged += new System.EventHandler(this.OnPanFactorValueChanged);
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(120, 176);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(32, 24);
            this.label7.TabIndex = 14;
            this.label7.Text = "Pan:";
            this.label7.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
            // 
            // controlPanelApplyButton
            // 
            this.controlPanelApplyButton.DialogResult = System.Windows.Forms.DialogResult.Yes;
            this.controlPanelApplyButton.Location = new System.Drawing.Point(85, 264);
            this.controlPanelApplyButton.Name = "controlPanelApplyButton";
            this.controlPanelApplyButton.Size = new System.Drawing.Size(56, 23);
            this.controlPanelApplyButton.TabIndex = 17;
            this.controlPanelApplyButton.Text = "Apply";
            this.controlPanelApplyButton.Click += new System.EventHandler(this.OnApplyClick);
            // 
            // ControlPanel
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(226, 296);
            this.Controls.Add(this.controlPanelApplyButton);
            this.Controls.Add(this.panFactorUpDown);
            this.Controls.Add(this.alwaysOnTopCHK);
            this.Controls.Add(this.navMapPB);
            this.Controls.Add(this.imageHeightTB);
            this.Controls.Add(this.imageWidthTB);
            this.Controls.Add(this.centerLatTB);
            this.Controls.Add(this.centerLonTB);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.scaleCB);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.typeCB);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.controlPanelCancelBtn);
            this.Controls.Add(this.controlPanelOKBtn);
            this.Controls.Add(this.label7);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "ControlPanel";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Control Panel";
            this.Load += new System.EventHandler(this.ControlPanel_Load);
            ((System.ComponentModel.ISupportInitialize)(this.panFactorUpDown)).EndInit();
            this.ResumeLayout(false);

        }
		#endregion

        private double CenterLon {
            get { return mainForm.CenterLon; }
            set { mainForm.CenterLon=value; centerLonTB.Text=mainForm.CenterLon.ToString(); }
        }
        
        private double CenterLat {
            get { return mainForm.CenterLat; }
            set { mainForm.CenterLat=value; centerLatTB.Text=mainForm.CenterLat.ToString(); }
        }
        
        private Int32 ImageWidth {
            get { return mainForm.ImageWidth; }
            set { mainForm.ImageWidth=value; imageWidthTB.Text=mainForm.ImageWidth.ToString(); }
        }
 
        private Int32 ImageHeight {
            get { return mainForm.ImageHeight; }
            set { mainForm.ImageHeight=value; imageHeightTB.Text=mainForm.ImageHeight.ToString(); }
        }
        
        private MapType MapType {
            get { return mainForm.MapType; }
            set { mainForm.MapType=value; typeCB.Text=mainForm.MapType.ToString(); }
        }

        private new TerraView.com.msrmaps.Scale Scale {
            get { return mainForm.Scale; }
            set { mainForm.Scale=value; scaleCB.Text=mainForm.Scale.ToString(); }
        }

        private Decimal PanFactor {
            get { return mainForm.PanFactor; }
            set { mainForm.PanFactor=value; panFactorUpDown.Value=mainForm.PanFactor; }
        }

        private Int32 ImageWidthDefault {
            get { return mainForm.ImageWidthDefault; }
        }
 
        private Int32 ImageHeightDefault {
            get { return mainForm.ImageHeightDefault; }
        }

        private MapType MapTypeDefault {
            get { return mainForm.MapTypeDefault; }
        }

        private Scale ScaleMin {
            get { return mainForm.ScaleMin; }
        }

        private Scale ScaleMax {
            get { return mainForm.ScaleMax; }
        }

        private Scale ScaleDefault {
            get { return mainForm.ScaleDefault; }
        }

        private void MakeHitRegions() {
            Point[] polyPoints;
            GraphicsPath path;

            // N
            polyPoints=new Point[]{
                                      new Point(43,31),
                                      new Point(50,1), 
                                      new Point(57,31),
            };
            path=new GraphicsPath();
            path.AddPolygon(polyPoints);
            north=new Region(path);
            // W
            polyPoints=new Point[]{
                                      new Point(31,57),
                                      new Point(0,49), 
                                      new Point(31,42),
            };
            path=new GraphicsPath();
            path.AddPolygon(polyPoints);
            west=new Region(path);
            // E
            polyPoints=new Point[]{
                                      new Point(69,44),
                                      new Point(99,51), 
                                      new Point(69,59),
            };
            path=new GraphicsPath();
            path.AddPolygon(polyPoints);
            east=new Region(path);
            // S
            polyPoints=new Point[]{
                                      new Point(43,69),
                                      new Point(57,69), 
                                      new Point(50,98),
            };
            path=new GraphicsPath();
            path.AddPolygon(polyPoints);
            south=new Region(path);
            // NE
            polyPoints=new Point[]{
                                      new Point(58,32),
                                      new Point(77,22), 
                                      new Point(68,43),
            };
            path=new GraphicsPath();
            path.AddPolygon(polyPoints);
            northEast=new Region(path);
            //SE
            polyPoints=new Point[]{
                                      new Point(67,59),
                                      new Point(76,77), 
                                      new Point(57,68),
            };
            path=new GraphicsPath();
            path.AddPolygon(polyPoints);
            southEast=new Region(path);
            // SW
            polyPoints=new Point[]{
                                      new Point(31,59),
                                      new Point(42,69), 
                                      new Point(23,77),
            };
            path=new GraphicsPath();
            path.AddPolygon(polyPoints);
            southWest=new Region(path);
            // NW
            polyPoints=new Point[]{
                                      new Point(22,22),
                                      new Point(42,31), 
                                      new Point(31,42),
            };
            path=new GraphicsPath();
            path.AddPolygon(polyPoints);
            northWest=new Region(path);
            // Zoom in
            polyPoints=new Point[]{
                                      new Point(42,33),
                                      new Point(48,33), 
                                      new Point(48,67),
                                      new Point(42,67),
                                      new Point(33,58),
                                      new Point(33,43),
            };
            path=new GraphicsPath();
            path.AddPolygon(polyPoints);
            zoomIn=new Region(path);
            // Zoom out
            polyPoints=new Point[]{
                                      new Point(50,33),
                                      new Point(58,33), 
                                      new Point(67,43),
                                      new Point(67,58),
                                      new Point(58,67), 
                                      new Point(50,67),
            };
            path=new GraphicsPath();
            path.AddPolygon(polyPoints);
            zoomOut=new Region(path);
        }

        public void Reset() {
            centerLonTB.Text=CenterLon.ToString();
            centerLatTB.Text=CenterLat.ToString();
            imageWidthTB.Text=ImageWidth.ToString();
            imageHeightTB.Text=ImageHeight.ToString();
            typeCB.Text=MapType.ToString();
            scaleCB.Text=Scale.ToString();
            panFactorUpDown.Value=PanFactor;
        }       
        
        private void ControlPanel_Load(object sender, System.EventArgs e) {
            // Fill in type combobox
            typeCB.Items.Add(MapType.Photo);
            typeCB.Items.Add(MapType.Topo);
            typeCB.Items.Add(MapType.UrbanAreas);
            typeCB.Items.Add(MapType.MapPoint);
#if false
            typeCB.Items.Add(MapType.Relief);
#endif            
            // Fill in scale combobox
            // Scale starts at Scale1mm and goes to Scale16km but only works for
            // Scale1m < Photo < Scale64m
            // Scale2m < Topo < Scale2km
            // MapPoint cannot get the lat/lon for scale=16km (globe)

            // Limit it to Scale250mm to Scale2km (of Scale1mm to Scale2048km)
            // (Based on debug code tests in Main.cs)
            for(Scale s=ScaleMin; s <= ScaleMax; s++) {
                scaleCB.Items.Add(s);
            }

            // Set the always-on-top check box
            this.TopMost=alwaysOnTop;
            alwaysOnTopCHK.Checked=alwaysOnTop;
        }

        private void OnOkClick(object sender, System.EventArgs e) {
            bool error=false;
            bool showError=true;
            bool reset=true;

            // Get the current paramaters
            GetParameters();

            // Check the scale
            if(mainForm.CheckScale(showError,reset)) error=true;
            
            CenterLon=System.Convert.ToDouble(centerLonTB.Text);
            CenterLat=System.Convert.ToDouble(centerLatTB.Text);
            ImageWidth=System.Convert.ToInt32(imageWidthTB.Text);
            ImageHeight=System.Convert.ToInt32(imageHeightTB.Text);

            // Insure non-zero values
            if(ImageWidth <= 1) {
                ImageWidth=ImageWidthDefault;
                ErrMsg.Show("Invalid width changed to " + ImageWidthDefault);
                error=true;
            }
            if(ImageHeight <= 1) {
                ImageHeight=ImageHeightDefault;
                ErrMsg.Show("Invalid height changed to " + ImageHeightDefault);
                error=true;
            }

            // Reset the control panel
            Reset();

            // Get the image unless there was an error
            if(!error && mainForm != null) {
                mainForm.Reset();
            }
        }

        private void GetParameters() {
            // Type
            if(typeCB.Text == MapType.Photo.ToString()) MapType=MapType.Photo;
            if(typeCB.Text == MapType.Topo.ToString()) MapType=MapType.Topo;
            if(typeCB.Text == MapType.UrbanAreas.ToString()) MapType=MapType.UrbanAreas;
            if(typeCB.Text == MapType.MapPoint.ToString()) MapType=MapType.MapPoint;

            // Scale (Leave all the scale choices here)
            for(Scale s=Scale.Scale1mm; s <= Scale.Scale2048km; s++) {
                if(scaleCB.Text == s.ToString()) Scale=s;
            }

            CenterLon=System.Convert.ToDouble(centerLonTB.Text);
            CenterLat=System.Convert.ToDouble(centerLatTB.Text);
            ImageWidth=System.Convert.ToInt32(imageWidthTB.Text);
            ImageHeight=System.Convert.ToInt32(imageHeightTB.Text);
        }

        private void OnCancelClick(object sender, System.EventArgs e) {
            Hide();
        }

        private void OnNavMouseDown(object sender, System.Windows.Forms.MouseEventArgs e) {
            bool debug=false;
            bool error=false;

            // Get the current parameters
            GetParameters();

            // Get the pan factor
            PanFactor=panFactorUpDown.Value;
            double factor=Decimal.ToDouble(PanFactor);
            if(debug) InfoMsg.Show("Pan Factor = " + factor);

            // Determine the hit point
            Point hitPt=new Point(e.X,e.Y);
            if(north.IsVisible(hitPt)) {
                if(debug) InfoMsg.Show("N");
                CenterLat+=factor*mainForm.DeltaLonLat.Lat;
            } else if(west.IsVisible(hitPt)) {
                if(debug) InfoMsg.Show("W");
                CenterLon-=factor*mainForm.DeltaLonLat.Lon;
            } else if(east.IsVisible(hitPt)) {
                if(debug) InfoMsg.Show("E");
                CenterLon+=factor*mainForm.DeltaLonLat.Lon;
            } else if(south.IsVisible(hitPt)) {
                if(debug) InfoMsg.Show("S");
                CenterLat-=factor*mainForm.DeltaLonLat.Lat;
            } else if(northEast.IsVisible(hitPt)) {
                if(debug) InfoMsg.Show("NE");
                CenterLat+=factor*mainForm.DeltaLonLat.Lat;
                CenterLon+=factor*mainForm.DeltaLonLat.Lon;
            } else if(southEast.IsVisible(hitPt)) {
                if(debug) InfoMsg.Show("SE");
                CenterLon+=factor*mainForm.DeltaLonLat.Lon;
                CenterLat-=factor*mainForm.DeltaLonLat.Lat;
            } else if(southWest.IsVisible(hitPt)) {
                if(debug) InfoMsg.Show("SW");
                CenterLon-=factor*mainForm.DeltaLonLat.Lon;
                CenterLat-=factor*mainForm.DeltaLonLat.Lat;
            } else if(northWest.IsVisible(hitPt)) {
                if(debug) InfoMsg.Show("NW");
                CenterLat+=factor*mainForm.DeltaLonLat.Lat;
                CenterLon-=factor*mainForm.DeltaLonLat.Lon;
            } else if(zoomIn.IsVisible(hitPt)) {
                if(debug) InfoMsg.Show("In");
                error=mainForm.ZoomIn();
            } else if(zoomOut.IsVisible(hitPt)) {
                if(debug) InfoMsg.Show("Out");
                error=mainForm.ZoomOut();
            } else {
                if(debug) InfoMsg.Show("Miss");
                error=true;
            }
            
            // Reset the control panel
            Reset();

            // Get the image unless there was an error
            if(!error && mainForm != null) {
                mainForm.Reset();
            }
        }

        private void OnAlwaysOnTopChanged(object sender, System.EventArgs e) {
            alwaysOnTop=alwaysOnTopCHK.Checked;
            this.TopMost=alwaysOnTop;
        }

        private void OnPanFactorValueChanged(object sender, System.EventArgs e) {
            PanFactor=panFactorUpDown.Value;        
        }

        private void OnApplyClick(object sender, System.EventArgs e) {
            GetParameters();
        }
	}
}
