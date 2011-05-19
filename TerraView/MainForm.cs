#undef DEBUG_PIXELSIZE
#undef USE_MAPPOINT
#undef QUICK_START

using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;
using System.Data;

using System.Drawing.Imaging;
using System.IO;
using TerraView.com.msrmaps;
using KEUtilities;
#if USE_MAPPOINT
using MapPointUtilities;
#endif

namespace TerraView {

    public enum MapType {
        Photo=1,      // Old Theme.Photo
        Topo=2,       // Old Theme.Topo
        UrbanAreas=4,  // Old Theme.Relief
        MapPoint=8,
    }

    /// <summary>
    /// Summary description for MainForm.
    /// </summary>
    public class MainForm : System.Windows.Forms.Form {
        private bool initialized=false;
        private Double[] scale2Alt;
        private bool processing=false;
        private bool listExceptions=true;
        private ControlPanel controlPanel=null;
        private ProgressDlg pDlg=new ProgressDlg();
        private String title="Terra View";
        private TerraService ts=null;
#if USE_MAPPOINT
        private MapPointClient mpc=null;
#endif
        private Graphics graphics=null;

    // Used by controlPanel
        // Argonne coordinates (center of APS)
        private double centerLon=-87.988130454, centerLat=41.703503930;

        private Int32 imageWidthDefault=800, imageHeightDefault=600;
        private Int32 imageWidth=800, imageHeight=600;

        private MapType mapTypeDefault=MapType.Photo;
        private MapType mapType=MapType.Photo;

        // Limit it to Scale250mm to Scale2km (of Scale1mm to Scale2048km)
        // (Based on debug code tests in Main.cs)
        private Scale scaleMin=Scale.Scale250mm;
        private Scale scaleMax=Scale.Scale2km;

        private Scale scaleDefault=Scale.Scale8m;
        private Scale scale=Scale.Scale8m;
        
        private Decimal panFactor=0.9M;

        private LonLatPt northEast,southEast,southWest,northWest,deltaLonLat;
        private String fileName=null;
        private String gpslFileName=null;
        private ImageFormat imageFormat=ImageFormat.Bmp;
        private double lon0=0.0,lonX=0.0,lonY=0.0,lonXY=0.0;
        private double lat0=0.0,latX=0.0,latY=0.0,latXY=0.0;
#if USE_MAPPOINT
        private bool gettingMapPointMap=false;
        private const double PixMapPoint2PixTerra=0.3857;
#endif

        private System.ComponentModel.Container components = null;
        private System.Windows.Forms.MenuItem fileMenu;
        private System.Windows.Forms.MenuItem fileMenuOpen;
        private System.Windows.Forms.MenuItem fileMenuExit;
        private System.Windows.Forms.MenuItem optionsMenu;
        private System.Windows.Forms.MenuItem optionsFitToImage;
        private System.Windows.Forms.MenuItem helpMenu;
        private System.Windows.Forms.MenuItem helpMenuAbout;
        private System.Windows.Forms.MenuItem optionsControlPanel;
        private System.Windows.Forms.MenuItem optionsSearch;
        private System.Windows.Forms.MenuItem fileMenuSave;
        private System.Windows.Forms.MenuItem fileMenuSaveAs;
        private System.Windows.Forms.MenuItem fileMenuSaveGpsl;
        private System.Windows.Forms.MenuItem fileMenuSaveAsGpsl;
        private System.Windows.Forms.StatusBar statusBar;
        private System.Windows.Forms.Panel mainPanel;
        private System.Windows.Forms.StatusBarPanel statusBarPanel1;
        private System.Windows.Forms.MainMenu mainMenu;
        private System.Windows.Forms.MenuItem optionsGetMapPointMap;
        private System.Windows.Forms.MenuItem optionsRefresh;
        private System.Windows.Forms.MenuItem helpMenuOverview;
        private System.Windows.Forms.PictureBox imagePictureBox;

        public MainForm() {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            //
            // TODO: Add any constructor code after InitializeComponent call
            //

            // Resize the panel
            OnFormResize(null,EventArgs.Empty);

            Text=title;
            deltaLonLat=new LonLatPt();
            deltaLonLat.Lon=deltaLonLat.Lat=0.0;

            // Make the TerraService
            try {
                ts=new TerraService();
            } catch(Exception e) {
                if(listExceptions) {
                    ExcMsg.Show("Cannot contact TerraService",e);
                } else {
                    ErrMsg.Show("Cannot contact TerraService");
                }
                return;
            }

            // Make the altitude scale table
            MakeAltitudeScale();

            // Set default size of imagePictureBox
            imagePictureBox.Size = new System.Drawing.Size(imageWidth,imageHeight);
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose( bool disposing ) {
            if( disposing ) {
#if USE_MAPPOINT
                if(mpc != null) {
                    try {
                        mpc.Dispose();
                        mpc=null;
                    } catch {
                    }
                }
#endif
                if (components != null) {
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
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(MainForm));
            this.mainMenu = new System.Windows.Forms.MainMenu();
            this.fileMenu = new System.Windows.Forms.MenuItem();
            this.fileMenuOpen = new System.Windows.Forms.MenuItem();
            this.fileMenuSave = new System.Windows.Forms.MenuItem();
            this.fileMenuSaveGpsl = new System.Windows.Forms.MenuItem();
            this.fileMenuSaveAs = new System.Windows.Forms.MenuItem();
            this.fileMenuSaveAsGpsl = new System.Windows.Forms.MenuItem();
            this.fileMenuExit = new System.Windows.Forms.MenuItem();
            this.optionsMenu = new System.Windows.Forms.MenuItem();
            this.optionsFitToImage = new System.Windows.Forms.MenuItem();
            this.optionsSearch = new System.Windows.Forms.MenuItem();
            this.optionsRefresh = new System.Windows.Forms.MenuItem();
            this.optionsGetMapPointMap = new System.Windows.Forms.MenuItem();
            this.optionsControlPanel = new System.Windows.Forms.MenuItem();
            this.helpMenu = new System.Windows.Forms.MenuItem();
            this.helpMenuOverview = new System.Windows.Forms.MenuItem();
            this.helpMenuAbout = new System.Windows.Forms.MenuItem();
            this.statusBar = new System.Windows.Forms.StatusBar();
            this.statusBarPanel1 = new System.Windows.Forms.StatusBarPanel();
            this.mainPanel = new System.Windows.Forms.Panel();
            this.imagePictureBox = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.statusBarPanel1)).BeginInit();
            this.mainPanel.SuspendLayout();
            this.SuspendLayout();
            // 
            // mainMenu
            // 
            this.mainMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.fileMenu,
            this.optionsMenu,
            this.helpMenu});
            // 
            // fileMenu
            // 
            this.fileMenu.Index = 0;
            this.fileMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.fileMenuOpen,
            this.fileMenuSave,
            this.fileMenuSaveGpsl,
            this.fileMenuSaveAs,
            this.fileMenuSaveAsGpsl,
            this.fileMenuExit});
            this.fileMenu.Text = "File";
            // 
            // fileMenuOpen
            // 
            this.fileMenuOpen.Index = 0;
            this.fileMenuOpen.Text = "Open...";
            this.fileMenuOpen.Click += new System.EventHandler(this.MainMenuHandler);
            // 
            // fileMenuSave
            // 
            this.fileMenuSave.Index = 1;
            this.fileMenuSave.Text = "Save";
            this.fileMenuSave.Click += new System.EventHandler(this.MainMenuHandler);
            // 
            // fileMenuSaveGpsl
            // 
            this.fileMenuSaveGpsl.Index = 2;
            this.fileMenuSaveGpsl.Text = "Save GPSL";
            this.fileMenuSaveGpsl.Click += new System.EventHandler(this.MainMenuHandler);
            // 
            // fileMenuSaveAs
            // 
            this.fileMenuSaveAs.Index = 3;
            this.fileMenuSaveAs.Text = "Save As...";
            this.fileMenuSaveAs.Click += new System.EventHandler(this.MainMenuHandler);
            // 
            // fileMenuSaveAsGpsl
            // 
            this.fileMenuSaveAsGpsl.Index = 4;
            this.fileMenuSaveAsGpsl.Text = "Save As GPSL...";
            this.fileMenuSaveAsGpsl.Click += new System.EventHandler(this.MainMenuHandler);
            // 
            // fileMenuExit
            // 
            this.fileMenuExit.Index = 5;
            this.fileMenuExit.Text = "Exit";
            this.fileMenuExit.Click += new System.EventHandler(this.MainMenuHandler);
            // 
            // optionsMenu
            // 
            this.optionsMenu.Index = 1;
            this.optionsMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.optionsFitToImage,
            this.optionsSearch,
            this.optionsRefresh,
            this.optionsGetMapPointMap,
            this.optionsControlPanel});
            this.optionsMenu.Text = "Options";
            // 
            // optionsFitToImage
            // 
            this.optionsFitToImage.Index = 0;
            this.optionsFitToImage.Text = "Fit To Image";
            this.optionsFitToImage.Click += new System.EventHandler(this.MainMenuHandler);
            // 
            // optionsSearch
            // 
            this.optionsSearch.Index = 1;
            this.optionsSearch.Text = "Search...";
            this.optionsSearch.Click += new System.EventHandler(this.MainMenuHandler);
            // 
            // optionsRefresh
            // 
            this.optionsRefresh.Index = 2;
            this.optionsRefresh.Text = "Refresh";
            this.optionsRefresh.Click += new System.EventHandler(this.MainMenuHandler);
            // 
            // optionsGetMapPointMap
            // 
            this.optionsGetMapPointMap.Index = 3;
            this.optionsGetMapPointMap.Text = "Get MapPoint Map";
            this.optionsGetMapPointMap.Click += new System.EventHandler(this.MainMenuHandler);
            // 
            // optionsControlPanel
            // 
            this.optionsControlPanel.Index = 4;
            this.optionsControlPanel.Text = "ControlPanel...";
            this.optionsControlPanel.Click += new System.EventHandler(this.MainMenuHandler);
            // 
            // helpMenu
            // 
            this.helpMenu.Index = 2;
            this.helpMenu.MenuItems.AddRange(new System.Windows.Forms.MenuItem[] {
            this.helpMenuOverview,
            this.helpMenuAbout});
            this.helpMenu.Text = "Help";
            // 
            // helpMenuOverview
            // 
            this.helpMenuOverview.Index = 0;
            this.helpMenuOverview.Text = "Overview...";
            this.helpMenuOverview.Click += new System.EventHandler(this.MainMenuHandler);
            // 
            // helpMenuAbout
            // 
            this.helpMenuAbout.Index = 1;
            this.helpMenuAbout.Text = "About...";
            this.helpMenuAbout.Click += new System.EventHandler(this.MainMenuHandler);
            // 
            // statusBar
            // 
            this.statusBar.Location = new System.Drawing.Point(0, 315);
            this.statusBar.Name = "statusBar";
            this.statusBar.Panels.AddRange(new System.Windows.Forms.StatusBarPanel[] {
            this.statusBarPanel1});
            this.statusBar.ShowPanels = true;
            this.statusBar.Size = new System.Drawing.Size(376, 22);
            this.statusBar.TabIndex = 1;
            // 
            // statusBarPanel1
            // 
            this.statusBarPanel1.Text = "Welcome to Terra View";
            this.statusBarPanel1.Width = 250;
            // 
            // mainPanel
            // 
            this.mainPanel.AutoScroll = true;
            this.mainPanel.BackColor = System.Drawing.Color.Black;
            this.mainPanel.BorderStyle = System.Windows.Forms.BorderStyle.Fixed3D;
            this.mainPanel.Controls.Add(this.imagePictureBox);
            this.mainPanel.Location = new System.Drawing.Point(0, 0);
            this.mainPanel.Name = "mainPanel";
            this.mainPanel.Size = new System.Drawing.Size(376, 312);
            this.mainPanel.TabIndex = 2;
            // 
            // imagePictureBox
            // 
            this.imagePictureBox.Location = new System.Drawing.Point(0, 0);
            this.imagePictureBox.Name = "imagePictureBox";
            this.imagePictureBox.TabIndex = 2;
            this.imagePictureBox.TabStop = false;
            this.imagePictureBox.MouseMove += new System.Windows.Forms.MouseEventHandler(this.OnImageMouseMove);
            this.imagePictureBox.MouseDown += new System.Windows.Forms.MouseEventHandler(this.OnImageMouseDown);
            // 
            // MainForm
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.BackColor = System.Drawing.Color.White;
            this.ClientSize = new System.Drawing.Size(376, 337);
            this.Controls.Add(this.statusBar);
            this.Controls.Add(this.mainPanel);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Menu = this.mainMenu;
            this.Name = "MainForm";
            this.Text = "Terra View";
            this.Resize += new System.EventHandler(this.OnFormResize);
            this.Load += new System.EventHandler(this.OnFormLoad);
            ((System.ComponentModel.ISupportInitialize)(this.statusBarPanel1)).EndInit();
            this.mainPanel.ResumeLayout(false);
            this.ResumeLayout(false);

        }
		#endregion

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main() {
            Application.Run(new MainForm());
        }

        public double CenterLon {
            get { return centerLon; }
            set { centerLon=value; }
        }
        
        public double CenterLat {
            get { return centerLat; }
            set { centerLat=value; }
        }
        
        public Int32 ImageWidth {
            get { return imageWidth; }
            set { imageWidth=value; }
        }
 
        public Int32 ImageHeight {
            get { return imageHeight; }
            set { imageHeight=value; }
        }
        
        public MapType MapType {
            get { return mapType; }
            set { mapType=value; }
        }

        public Int32 ImageWidthDefault {
            get { return imageWidthDefault; }
        }
 
        public Int32 ImageHeightDefault {
            get { return imageHeightDefault; }
        }

        public MapType MapTypeDefault {
            get { return mapTypeDefault; }
        }

        public Scale ScaleMin {
            get { return scaleMin; }
        }

        public Scale ScaleMax {
            get { return scaleMax; }
        }

        public Scale ScaleDefault {
            get { return scaleDefault; }
        }

        public new TerraView.com.msrmaps.Scale Scale {
            get { return scale; }
            set { scale=value; }
        }

        public Decimal PanFactor {
            get { return panFactor; }
            set { panFactor=value; }
        }

        public LonLatPt DeltaLonLat {
            get { return deltaLonLat; }
            set { deltaLonLat=value; }
        }

        public TerraService TerraServer {
            get { return ts; }
        }

        private void MainMenuHandler(object sender, System.EventArgs e) {
#if false
            MenuItem item=(MenuItem)sender;
            InfoMsg.Show(item.Text + "\n     Index=" + item.Index +
                "\n     Handle=" + item.Handle);
#endif
            if(sender == fileMenuExit) {
                // FileMenuExit
                Application.Exit();
            } else if(sender == fileMenuOpen) {
                ErrMsg.Show("Open is not implemented yet");
            } else if(sender == fileMenuSave || sender == fileMenuSaveGpsl) {
                // FileMenuSave
                DoFileSave(sender);
            } else if(sender == fileMenuSaveAs || sender == fileMenuSaveAsGpsl) {
                DoFileSaveAs(sender);
            } else if(sender == optionsFitToImage) {
                // OptionsFitToImage
                Size statusHeight=new Size(0,statusBar.Height);
                mainPanel.ClientSize=imagePictureBox.Size;
                this.ClientSize=mainPanel.Size+statusHeight;
                Invalidate();
            } else if(sender == optionsSearch) {
                // Search dialog
                PlaceDlg placeDlg=new PlaceDlg(this,ts);
                placeDlg.ShowDialog();
                if(placeDlg.Valid) {
                    centerLon=placeDlg.CenterLon;
                    centerLat=placeDlg.CenterLat;
                    Reset();
                }
            } else if(sender == optionsRefresh) {
                // Refresh
                imagePictureBox.Invalidate();
                Invalidate();
            } else if(sender == optionsControlPanel) {
                // Options Control Panel
                if(controlPanel == null) {
                    controlPanel=new ControlPanel(this);
                    controlPanel.Disposed += new EventHandler(this.OnControlPanelDisposed);
                }
                controlPanel.Reset();
                controlPanel.Show();
#if USE_MAPPOINT
            } else if(sender == optionsGetMapPointMap) {
                if(mpc == null || !mpc.Started) {
                    ErrMsg.Show("MapPoint must be started from TerraView first");
                    return;
                }
                GetMapPointMap();
#endif
            } else if(sender == helpMenuOverview) {
                // HelpMenuOverview
                OverviewDlg dlg=new OverviewDlg();
                dlg.ShowDialog();
            } else if(sender == helpMenuAbout) {
                // HelpMenuAbout
                String extraText="The images are United States Geographical" +
                    " Society (USGS) images that are provided by the" +
                    " Microsoft TerraServer or by Microsoft MapPoint.";
                AboutDlg dlg=new AboutDlg(extraText);
                dlg.ShowDialog();
            }
        }

        private void DoFileSave(Object sender) {
            if(fileName == null) {
                ErrMsg.Show("No file to be saved\nUse SaveAs... first");
            } else {
                try {
                    imagePictureBox.Image.Save(fileName,imageFormat);
                } catch(Exception e) {
                    if(listExceptions) {
                        ExcMsg.Show("Error saving file:\n" + fileName,e);
                    } else {
                        ErrMsg.Show("Error saving file:\n" + fileName);
                    }
                    return;
                }
            }
            if(sender == fileMenuSaveGpsl) {
                // FileMenuSaveGpsl
                if(gpslFileName == null) {
                    ErrMsg.Show("No GPSL file to be saved\nUse SaveAs GPSL... first");
                } else {
                    DoSaveGpsl();
                }
            }
        }

        private void DoFileSaveAs(Object sender) {
            // FileMenuSaveAs
            SaveFileDialog saveFileDialog=new SaveFileDialog();
            saveFileDialog.Filter="JPEG (*.jpg;*.jpeg)|*.jpg;*.jpeg" +
                "|PNG (*.png)|*.png" +
                "|BMP (*.bmp)|*.bmp" +
                "|GIF (*.gif)|*.gif" +
                "|TIFF (*.tif;*.tiff)|*.tif;*.tiff" +
                "|All files (*.*)|*.*";
            saveFileDialog.CheckPathExists=true;
            if(saveFileDialog.ShowDialog() == DialogResult.OK) {
                try {
                    fileName=saveFileDialog.FileName;
                    if(saveFileDialog.FilterIndex == 1) {
                        imageFormat=ImageFormat.Jpeg;
                    } else if(saveFileDialog.FilterIndex == 2) {
                        imageFormat=ImageFormat.Png;
                    } else if(saveFileDialog.FilterIndex == 3) {
                        imageFormat=ImageFormat.Bmp;
                    } else if(saveFileDialog.FilterIndex == 4) {
                        imageFormat=ImageFormat.Gif;
                    } else if(saveFileDialog.FilterIndex == 5) {
                        imageFormat=ImageFormat.Tiff;
                    } else {
                        ErrMsg.Show("Pick JPEG, PNG, BMP, GIF, or TIFF");
                        return;
                    }
                    imagePictureBox.Image.Save(fileName,imageFormat);
                } catch(Exception e) {
                    if(listExceptions) {
                        ExcMsg.Show("Error saving file:\n" + fileName,e);
                    } else {
                        ErrMsg.Show("Error saving file:\n" + fileName);
                    }
                    return;
                }
                // Do the GPSL file in addition if specified
                if(sender == fileMenuSaveAsGpsl) {
                    // FileMenuSaveAsGpsl
                    int index=fileName.LastIndexOf('.');
                    if(index <= 0) {
                        ErrMsg.Show("Error making GPSL filename from\n" +
                            fileName);
                        return;
                    }
                    gpslFileName=fileName.Substring(0,index) +
                        ".map.gpsl";
                    saveFileDialog.Filter="GPSL (*.gpsl)|*.gpsl" +
                        "|All files (*.*)|*.*";
                    saveFileDialog.FileName=gpslFileName;
                    if(saveFileDialog.ShowDialog() == DialogResult.OK) {
                        gpslFileName=saveFileDialog.FileName;
                        DoSaveGpsl();
                    }
                }
            }
        }

        private void DoSaveGpsl() {
            try {
                FileStream fs = new FileStream(gpslFileName,
                    FileMode.Create, FileAccess.Write, FileShare.None);
                StreamWriter sw=new StreamWriter(fs,System.Text.Encoding.ASCII);
                String gpsLinkId="!GPSLINK";
                sw.WriteLine(gpsLinkId);
                DateTime now=DateTime.Now;
                DateTime nowGmt=now.ToUniversalTime();
                long gmtOffset=now.Hour-nowGmt.Hour;
                sw.WriteLine("Saved " + now);
                String tab="\t";
                sw.WriteLine("Delimiter=" + tab);
                sw.WriteLine("GMTOffset=" + gmtOffset);
                sw.WriteLine("");
                sw.WriteLine("Map");
                sw.WriteLine("# " + this.Text);
                // Use the short path name
                sw.WriteLine("M" + tab + Path.GetFileName(fileName));
                sw.WriteLine("C\tNW\t{0:f8}\t{1:f8}\t{2}\t{3}",
                    northWest.Lat,northWest.Lon,0,0);
                sw.WriteLine("C\tNE\t{0:f8}\t{1:f8}\t{2}\t{3}",
                    northEast.Lat,northEast.Lon,imageWidth-1,0);
                sw.WriteLine("C\tSE\t{0:f8}\t{1:f8}\t{2}\t{3}",
                    southEast.Lat,southEast.Lon,imageWidth-1,imageHeight-1);
                sw.WriteLine("C\tSW\t{0:f8}\t{1:f8}\t{2}\t{3}",
                    southWest.Lat,southWest.Lon,0,imageHeight-1);
                sw.Close();
            } catch(Exception e) {
                if(listExceptions) {
                    ExcMsg.Show("Error writing GPSL file\n" +
                        gpslFileName,e);
                } else {
                    ErrMsg.Show("Error writing GPSL file\n" +
                        gpslFileName);
                }
                return;
            }
        }

        public void Reset() {
            // Return if we are already processing
            if(processing) return;
            if(initialized) processing=true;

            if(controlPanel != null) {
                controlPanel.Reset();
            }

            // Call the appropriate reset
            
            // Specify the picture box
            try {
                imagePictureBox.Size = new System.Drawing.Size(imageWidth,imageHeight);
                imagePictureBox.SizeMode=PictureBoxSizeMode.AutoSize;
                PixelFormat pixelFormat=PixelFormat.Format32bppRgb;
                imagePictureBox.Image=new Bitmap(imageWidth,imageHeight,pixelFormat);
                graphics=Graphics.FromImage(imagePictureBox.Image);
            } catch(Exception e) {
                if(listExceptions) {
                    ExcMsg.Show("Cannot make an image",e);
                } else {
                    ErrMsg.Show("Cannot make an image");
                }
                processing=false;
                return;
            }

            // Return if not initilaized
            if(!initialized) {
                initialized=true;
                return;
            }

            // Get the image
            try {
                if(mapType == MapType.MapPoint) {
#if USE_MAPPOINT
                    Cursor.Current=Cursors.WaitCursor;
                    GetMapPointImage();
#endif
                } else {
                    GetTerraImage();
                }
            } catch(Exception e) {
                if(listExceptions) {
#if false
                    String temp="The TextBox control allows the user to enter text " +
                    "in an application. This control has additional functionality " +
                    "that is not found in the standard Windows text box control, " +
                    "including multiline editing and password character masking.  " +
                    "Typically, a TextBox control is used to display, or accept as " +
                    "input, a single line of text. You can use the Multiline and " +
                    "ScrollBars properties to enable multiple lines of text to be " +
                    "displayed or entered. Set the AcceptsTab and AcceptsReturn " +
                    "properties to true to allow greater text manipulation in a " +
                    "multiline TextBox control.  You can limit the amount of text " +
                    "entered into a TextBox control by setting the MaxLength property " +
                    "to a specific number of characters. TextBox controls can also be " +
                    "used to accept passwords and other sensitive information. You " +
                    "can use the PasswordChar property to mask characters entered in " +
                    "a single-line version of the control.  To restrict text from " +
                    "being entered in a TextBox control, you can create an event " +
                    "handler for the KeyDown event in order to validate each " +
                    "character entered in the control. You can also restrict all " +
                    "entry of data in a TextBox control by setting the ReadOnly " +
                    "property to true.  Note   Most of the functionality of the " +
                    "TextBox control is inherited from the TextBoxBase class.";
                    ExcMsg.ShowDlg("Failed to get image" + ". " + temp,e);
#else
                    ExcMsg.ShowDlg("Failed to get image",e);
#endif
                } else {
                    ErrMsg.Show("Failed to get image");
                }
            } finally {
                // It seems to be necessary to invalidate the picture box
                imagePictureBox.Invalidate();
                // Invalidate the form
                Invalidate();
                processing=false;
                Cursor.Current=Cursors.Default;
            }
        }
#if USE_MAPPOINT
        private void GetMapPointImage() {
            if(mpc == null) {
                try {
                    mpc=new MapPointClient();
                } catch(Exception e) {
                    if(listExceptions) {
                        ExcMsg.ShowDlg("Cannot start MapPointClient",e);
                    } else {
                        ErrMsg.Show("Cannot start MapPointClient");
                    }
                    return;
                }
            }
            if(!mpc.Started) {
                try {
                    mpc.Start();
                } catch(Exception e) {
                    if(listExceptions) {
                        ExcMsg.ShowDlg("Cannot start MapPoint",e);
                    } else {
                        ErrMsg.Show("Cannot start MapPoint");
                    }
                    return;
                }
            }
            try {
                mpc.Width=imageWidth;
                mpc.Height=imageHeight;
            } catch(Exception e) {
                if(listExceptions) {
                    ExcMsg.Show("Error setting MapPoint size",e);
                } else {
                    ErrMsg.Show("Error setting MapPoint size");
                }
                return;
            }
            try {
                if(!gettingMapPointMap) {
                    double altitude=GetAltFromScale(scale);
                    MapPoint.Location loc=mpc.Map.GetLocation(centerLat,centerLon,altitude);
                    mpc.Map.Location=loc;
                }
            } catch(Exception e) {
                if(listExceptions) {
                    ExcMsg.Show("Error setting MapPoint map",e);
                } else {
                    ErrMsg.Show("Error setting MapPoint map");
                }
                return;
            }
            try {
                mpc.Map.CopyMap();
                IDataObject ido=Clipboard.GetDataObject();
                if(ido.GetDataPresent(DataFormats.Bitmap)) {
                    Image clipImage=(Bitmap)ido.GetData(DataFormats.Bitmap);
                    graphics.DrawImage(clipImage,0,0,clipImage.Width,clipImage.Height);
                } else {
                    ErrMsg.Show("Error getting MapPoint image");
                    return;
                }
            } catch(Exception e) {
                if(listExceptions) {
                    ExcMsg.Show("Error getting MapPoint map",e);
                } else {
                    ErrMsg.Show("Error getting MapPoint map");
                }
                return;
            }
            try {
#if DEBUG_PIXELSIZE
                double pixelSize=mpc.Map.PixelSize*621.37;
                Text=title + ": PixelSize=" + pixelSize.ToString("f6") +
                    ", Altitude=" + mpc.Map.Altitude +
                    ", scale2Alt=" + scale2Alt[(int)scale] +
                    ", scale=" + (int)scale;
#else
                Text=title + ": " + mpc.Map.Location.PlaceCategory.Name;
#endif
                } catch {
                Text=title + ": MapPoint Map";
            }
#if true
            int x0=mpc.Map.Left;
            int y0=mpc.Map.Top;
            MapPoint.Location nwLoc=mpc.Map.XYToLocation(0,0);
            MapPoint.Location neLoc=mpc.Map.XYToLocation(imageWidth-1,0);
            MapPoint.Location seLoc=mpc.Map.XYToLocation(imageWidth-1,imageHeight-1);
            MapPoint.Location swLoc=mpc.Map.XYToLocation(0,imageHeight-1);
            MapPointClient.LonLatPtD nwPt=mpc.GetLatLonFromLocation(nwLoc);
            MapPointClient.LonLatPtD nePt=mpc.GetLatLonFromLocation(neLoc);
            MapPointClient.LonLatPtD sePt=mpc.GetLatLonFromLocation(seLoc);
            MapPointClient.LonLatPtD swPt=mpc.GetLatLonFromLocation(swLoc);
            if(northWest == null) northWest=new LonLatPt();
            if(northEast == null) northEast=new LonLatPt();
            if(southEast == null) southEast=new LonLatPt();
            if(southWest == null) southWest=new LonLatPt();
            northWest.Lat=nwPt.Lat;
            northWest.Lon=nwPt.Lon;
            northEast.Lat=nePt.Lat;
            northEast.Lon=nePt.Lon;
            southEast.Lat=sePt.Lat;
            southEast.Lon=sePt.Lon;
            southWest.Lat=swPt.Lat;
            southWest.Lon=swPt.Lon;
            GetTranslation();
#endif
        }
#endif

        private int MapTypeToTheme(MapType mapType) {
            if(mapType == MapType.Photo ||
                mapType == MapType.Topo ||
                mapType == MapType.UrbanAreas) {
                return (int)mapType;
            } else {
                ErrMsg.Show("Invalid Theme: " + mapType +
                    "\nUsing Photo");
                return (int)MapType.Photo;
            }
        }
        
        private void GetTerraImage() {
            // Get the area bounding box
            LonLatPt center=new LonLatPt();
            center.Lon=centerLon;
            center.Lat=centerLat;
            AreaBoundingBox abb=ts.GetAreaFromPt(center,MapTypeToTheme(mapType),
                scale,imageWidth,imageHeight);

            // Get the tiles
            Int32 xStart=abb.NorthWest.TileMeta.Id.X;
            Int32 yStart=abb.NorthWest.TileMeta.Id.Y;
            Int32 xEnd=abb.NorthEast.TileMeta.Id.X;
            Int32 yEnd=abb.SouthWest.TileMeta.Id.Y;
            pDlg.ItemsName="tiles";
            pDlg.Minimum=pDlg.Value=1;
            pDlg.Maximum=(xEnd-xStart)*(yStart-yEnd);
            pDlg.Step=1;
            pDlg.Reset();
            pDlg.Show();
            Application.DoEvents();

            for(Int32 x=xStart; x <= xEnd; x++) {
                for(Int32 y=yStart; y >= yEnd; y--) {
                    TileId tid=abb.NorthWest.TileMeta.Id;
                    tid.X=x;
                    tid.Y=y;
                    Image tileImage=Image.FromStream(new MemoryStream(ts.GetTile(tid)));
                    Int32 xPix=(x-xStart)*tileImage.Width-(Int32)abb.NorthWest.Offset.XOffset;
                    Int32 yPix=(yStart-y)*tileImage.Height-(Int32)abb.NorthWest.Offset.YOffset;
                    graphics.DrawImage(tileImage,xPix,yPix,tileImage.Width,tileImage.Height);
                    pDlg.PerformStep();
                    // Process any waiting events
                    Application.DoEvents();
                    if(pDlg.Cancelled) break;
                }
                if(pDlg.Cancelled) break;
            }
            pDlg.Hide();
            Application.DoEvents();

            // Set the title
            Text=title + ": " + abb.NearestPlace + " " +
                abb.Center.TileMeta.Capture.ToShortDateString();

            // Determine the corner longitude, latitude
            northEast=abb.NorthEast.Offset.Point;
            southEast=abb.SouthEast.Offset.Point;
            southWest=abb.SouthWest.Offset.Point;
            northWest=abb.NorthWest.Offset.Point;
            GetTranslation();
        }

        private void GetTranslation() {
            // Determine the translation coefficients
            try {
                lon0=northWest.Lon;
                lonX=(northEast.Lon-northWest.Lon)/(imageWidth-1);
                lonY=(southWest.Lon-northWest.Lon)/(imageHeight-1);
                lonXY=(southEast.Lon-southWest.Lon-northEast.Lon+northWest.Lon)/
                    ((imageWidth-1)*(imageHeight-1));
                lat0=northWest.Lat;
                latX=(northEast.Lat-northWest.Lat)/(imageWidth-1);
                latY=(southWest.Lat-northWest.Lat)/(imageHeight-1);
                latXY=(southEast.Lat-southWest.Lat-northEast.Lat+northWest.Lat)/
                    ((imageWidth-1)*(imageHeight-1));
#if DEBUG_PIXELSIZE
                double dTop=MapPointClient.GreatCircleDistance(
                    northWest.Lon,northWest.Lat,northEast.Lon,northEast.Lat);
                double dBottom=MapPointClient.GreatCircleDistance(
                    southWest.Lon,southWest.Lat,southEast.Lon,southEast.Lat);
                double dLeft=MapPointClient.GreatCircleDistance(
                    northWest.Lon,northWest.Lat,southWest.Lon,southWest.Lat);
                double dRight=MapPointClient.GreatCircleDistance(
                    northEast.Lon,northEast.Lat,southEast.Lon,southEast.Lat);
                double pixelSizeTop=dTop/imageWidth*MapPointClient.MI2M;
                double pixelSizeBottom=dBottom/imageWidth*MapPointClient.MI2M;
                double pixelSizeLeft=dLeft/imageHeight*MapPointClient.MI2M;
                double pixelSizeRight=dRight/imageHeight*MapPointClient.MI2M;

                Text=title + ": PixelSize=" +
                    pixelSizeTop.ToString("f4") + "," +
                    pixelSizeBottom.ToString("f4") + "," +
                    pixelSizeLeft.ToString("f4") + "," +
                    pixelSizeRight.ToString("f4") +
                    ", scale=" + scale;
#endif
            } catch(Exception e) {
                if(listExceptions) {
                    ExcMsg.Show("Error determining translation coordinates",e);
                }
                lon0=lonX=lonY=lonXY=0.0;
                lat0=latX=latY=latXY=0.0;
            }

            // Determine the increments
            deltaLonLat.Lon=.5*
                (northEast.Lon+southEast.Lon-northWest.Lon-southWest.Lon);
            deltaLonLat.Lat=.5*
                (northEast.Lat-southEast.Lat+northWest.Lat-southWest.Lat); 
        }

        private void MakeAltitudeScale() {
            scale2Alt=new Double[25];
            scale2Alt[0]=0.002071668;
            scale2Alt[1]=0.004143337;
            scale2Alt[2]=0.008286674;
            scale2Alt[3]=0.016573348;
            scale2Alt[4]=0.033146696;
            scale2Alt[5]=0.066293391;
            scale2Alt[6]=0.130515114;
            scale2Alt[7]=0.258958559;
            scale2Alt[8]=0.517917118;
            scale2Alt[9]=1.035834237;
            scale2Alt[10]=2.071668474;  // Scale1m
            scale2Alt[11]=4.143336948;  // Scale2m
            scale2Alt[12]=8.286673895;  // Scale4m
            scale2Alt[13]=16.57334779;  // Scale8m
            scale2Alt[14]=33.14669558;  // Scale16m
            scale2Alt[15]=66.29339116;  // Scale32m
            scale2Alt[16]=132.5867823;  // Scale64m
            scale2Alt[17]=265.1735647;  // Scale128m
            scale2Alt[18]=530.3471293;
            scale2Alt[19]=1060.694259;
            scale2Alt[20]=2071.668474;
            scale2Alt[21]=4143.336948;
            scale2Alt[22]=8286.673895;
            scale2Alt[23]=16573.34779;
            scale2Alt[24]=33146.69558;
        }

        private LonLatPt GetLonLatFromPixel(Point pt) {
            LonLatPt ret=new LonLatPt();
            double x=pt.X,y=pt.Y;
            ret.Lon=lon0+lonX*x+lonY*y+lonXY*x*y;
            ret.Lat=lat0+latX*x+latY*y+latXY*x*y;
            return ret;
        }

#if USE_MAPPOINT
        private double GetAltFromScale(Scale scale) {
            int intScale=(int)scale;
            double retVal;
            if(intScale <= 0) retVal=scale2Alt[0];
            else if(intScale >= 24) retVal=scale2Alt[24];
            else retVal=scale2Alt[intScale];
            return retVal*PixMapPoint2PixTerra;
        }
        private void GetMapPointMap() {
            try {
                gettingMapPointMap=true;
                imageWidth=mpc.Width;
                imageHeight=mpc.Height;
                MapPoint.Location loc=mpc.Map.Location;
                MapPointClient.LonLatPtD pt=mpc.GetLatLonFromLocation(loc);
                centerLat=pt.Lat;
                centerLon=pt.Lon;
                mapType=MapType.MapPoint;
                Reset();
            } catch(Exception e) {
                if(listExceptions) {
                    ExcMsg.Show("Error getting MapPoint map",e);
                } else {
                    ErrMsg.Show("Error getting MapPoint map");
                }
                return;
            } finally {
                gettingMapPointMap=false;
            }
        }
#endif
        public bool CheckScale(bool showError, bool reset) {
            bool error=false;

            // Check limits for this MapType
            if(MapType != MapType.MapPoint) {
                // Check TerraServer Limits
                ThemeInfo to=TerraServer.GetTheme((int)MapType);
                if(to == null) {
                    if(reset) MapType=MapTypeDefault;
                    error=true;
                } else if(scale < to.LoScale) {
                    if(showError) ErrMsg.Show("Minimum scale for " +
                                      mapType +  " is " + to.LoScale);
                    if(reset) scale=to.LoScale;
                    error=true;
                } else if(scale > to.HiScale) {
                    if(showError) ErrMsg.Show("Maximum scale for " +
                                      mapType +  " is " + to.HiScale);
                    if(reset) scale=to.HiScale;
                    error=true;
                }
            } else {
                // To Do
            }
            if(error == true) return error;

            // Check TerraView limits
            if(scale < scaleMin) {
                ErrMsg.Show("Minimum TerraView scale is " + scaleMin);
                if(reset) scale=scaleMin;
                error=true;
            } else if(scale > scaleMax) {
                ErrMsg.Show("Maximum TerraView scale is " + scaleMax);
                if(reset) scale=scaleMax;
                error=true;
            }
            return error;
        }

        public bool ZoomIn() {
            bool showError=true;
            bool reset=true;
            scale--;
            return CheckScale(showError,reset);
        }

        public bool ZoomOut() {
            bool showError=true;
            bool reset=true;
            scale++;
            return CheckScale(showError,reset);
        }

        private void OnFormResize(object sender, System.EventArgs e) {
            Size statusHeight=new Size(0,statusBar.Height);
            mainPanel.Size=this.ClientSize-statusHeight;
            Invalidate();
        }

        private void OnImageMouseDown(object sender, System.Windows.Forms.MouseEventArgs e) {
            bool debug=false;
            bool error=false;
            // Only respond to left click
            if(e.Button != MouseButtons.Left) return;

            // Get the modifier keys
            bool alt=((Control.ModifierKeys & Keys.Alt) == Keys.Alt);
            bool ctrl=((Control.ModifierKeys & Keys.Control) == Keys.Control);
            bool shift=((Control.ModifierKeys & Keys.Shift) == Keys.Shift);
            try {
                Point hitPt=new Point(e.X,e.Y);
                LonLatPt pt=GetLonLatFromPixel(hitPt);
                centerLon=pt.Lon;
                centerLat=pt.Lat;
#if true
                if(alt && !ctrl && !shift) {
                    // Zoom in
                    if(debug) InfoMsg.Show("Alt+Left");
                    error=ZoomIn();
                    controlPanel.Reset();
                } else if(alt && ctrl && !shift) {
                    // Zoom out
                    if(debug) InfoMsg.Show("Alt+Ctrl+Left");
                    error=ZoomOut();
                    controlPanel.Reset();
                } else if(alt && !ctrl && shift) {
                    // Zoom in 2x
                    if(debug) InfoMsg.Show("Alt+Shift+Left");
                    error=ZoomIn();
                    if(!error) {
                        ZoomIn();
                    }
                    controlPanel.Reset();
                } else if(alt && ctrl && shift) {
                    // Zoom out 2x
                    if(debug) InfoMsg.Show("Alt+Ctrl+Shift+Left");
                    error=ZoomOut();
                    if(!error) {
                        ZoomOut();
                    }
                    controlPanel.Reset();
                } else if(!alt && !ctrl && !shift) {
                    // No zoom
                } else {
                    return;
                }
#endif
                if(!error) Reset();
            } catch(Exception ex) {
                if(listExceptions) {
                    ExcMsg.Show("Error getting cursor position",ex);
                } else {
                    ErrMsg.Show("Error getting cursor position");
                }
            }
        }

        private void OnImageMouseMove(object sender, System.Windows.Forms.MouseEventArgs e) {
            Point hitPt=new Point(e.X,e.Y);
            LonLatPt pt=GetLonLatFromPixel(hitPt);
            statusBarPanel1.Text="Lon=" + pt.Lon.ToString("f6") +
                " Lat=" + pt.Lat.ToString("f6") +
                " x=" + hitPt.X.ToString() + " y=" + hitPt.Y.ToString();
        }

        private void OnFormLoad(object sender, System.EventArgs e) {
#if false
            // Test valid types
            int type;
            for(type=0; type < 256; type++) {
                ThemeInfo to=ts.GetTheme((int)type);
                if(to == null) {
                    ErrMsg.Show("Invalid type: " + type);
                } else if(to.Name != "" && to.LoScale != to.HiScale) {
                    ErrMsg.Show(type + ": Range for " + to.Name +  " is " +
                        to.LoScale + " to " + to.HiScale);
                }
            }
#endif
#if QUICK_START
            // Get default image
            initialized=true;
#else
#if false
            // If done here the search appears before the MainForm
            // Reset then show the search dialog
            imagePictureBox.Invalidate();
            Invalidate();
            bool valid=false;
            while(!valid) {
                PlaceDlg placeDlg=new PlaceDlg(this,ts);
                placeDlg.ShowDialog();
                if(placeDlg.DialogResult == DialogResult.Cancel) {
                    return;
                }
                if(placeDlg.Valid) {
                    centerLon=placeDlg.CenterLon;
                    centerLat=placeDlg.CenterLat;
                    valid=true;
                }
            }
#else
            if(controlPanel == null) {
                controlPanel=new ControlPanel(this);
                controlPanel.Disposed += new EventHandler(this.OnControlPanelDisposed);
            }
            controlPanel.Reset();
            controlPanel.Show();
#endif
#endif
            Reset();
        }

        private void OnControlPanelDisposed(object sender, System.EventArgs e) {
            // Set it to null
            controlPanel=null;
        }

    }
}
