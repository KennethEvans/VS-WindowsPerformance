using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace TerraView
{
	/// <summary>
	/// Summary description for OverviewDlg.
	/// </summary>
	public class OverviewDlg : System.Windows.Forms.Form
	{
        private System.Windows.Forms.TextBox textBox1;
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;
        String helpText=
        #region Help Text
            "			 TerraView\n" +
            "			 =========\n" +
            "\n" +
            "TerraView is an application to obtain images and display them.  The\n" +
            "images are United States Geographical Society (USGS) images that are\n" +
            "provided by the Microsoft TerraServer USA or by Microsoft MapPoint.\n" +
            "The Microsoft TerraServer is a free Web Service.  It requires an\n" +
            "Internet connection.  To get the MapPoint images you need to have\n" +
            "Microsoft MapPoint installed on your computer.\n" +
            "\n" +
            "When you start the program, the Control Panel will display.  You can\n" +
            "use it to choose the location by latitude and longitude and also vary\n" +
            "the parameters.  When you press the Go button on the Control Panel, it\n" +
            "will display the image.  If you want to search for a location by name\n" +
            "rather than by specifying the latitude and longitude, first select the\n" +
            "desired parameters on the Control Panel and select Apply.  Then select\n" +
            "Search on the Options Menu and find a location by typing its name.\n" +
            "When you select OK on the Search dialog, it will display the location\n" +
            "using the parameters you applied.  If you did not apply any, it will\n" +
            "use the default parameters.\n" +
            "\n" +
            "The image is displayed in the TerraView Main Window.  Clicking in the\n" +
            "Main Window re-centers the image.  The Control Panel has additional\n" +
            "controls for selecting the image and is described in more detail\n" +
            "below.  You can also use Search at any time as described above.\n" +
            "\n" +
            "You can also zoom as you click in the Main Window, using the following\n" +
            "modifier keys:\n" +
            "\n" +
            "	 Alt + Left\t\t\tZoom In\n" +
            "	 Ctrl + Alt + Left\t\tZoom Out\n" +
            "	 Shift + Alt + Left\t\tZoom In 2x\n" +
            "	 Shift + Ctrl + Alt + Left\t\tZoom Out 2x\n" +
            "\n" +
            "TerraView is distributed subject to the license conditions in the file\n" +
            "LICENSE included with the program.  The LICENSE file must always be\n" +
            "distributed with the program.\n" +
            "\n" +
            "File Menu\n" +
            "=========\n" +
            "\n" +
            "Open...\n" +
            "\n" +
            "Open is not implemented.\n" +
            "\n" +
            "Save\n" +
            "\n" +
            "Once an image has been saved once, this item saves it again,\n" +
            "overwriting the original.\n" +
            "\n" +
            "Save GPSL\n" +
            "\n" +
            "Once an image and the corresponding GPSL file have been saved via Save\n" +
            "GPSL, the two files will be saved again, overwriting the originals.\n" +
            "This allows you to quickly resave the current image, which is easily\n" +
            "opened in GPSLink to replace the current image in the Map Window.\n" +
            "\n" +
            "Save As...\n" +
            "\n" +
            "This item allows you to save the current image as a JPEG, PNG, BMP,\n" +
            "GIF or TIFF image, prompting for a filename.\n" +
            "\n" +
            "Save As GPSL...\n" +
            "\n" +
            "This item first allows you to save an image file, the same as for Save\n" +
            "As, and then allows you to save an additional file in GPSL format that\n" +
            "references the image file.  The second file has a .gpsl extension.\n" +
            "The two files can then be used in GPSLink.  Opening the GPSL file in\n" +
            "GPSLink will load the image into the Map Window in GPSLink and the\n" +
            "image will already be properly calibrated.\n" +
            "\n" +
            "Exit\n" +
            "\n" +
            "This item closes TerraView.\n" +
            "\n" +
            "Options Menu\n" +
            "============\n" +
            "\n" +
            "Fit to Image\n" +
            "\n" +
            "Resizes the TerraView Main Window to fit the current image.\n" +
            "\n" +
            "Search...\n" +
            "\n" +
            "Allows you to search for a location.\n" +
            "\n" +
            "Refresh\n" +
            "\n" +
            "Refreshes the TerraView Main Window.\n" +
            "\n" +
            "Get MapPoint Map\n" +
            "\n" +
            "If MapPoint is running, displays its current map in TerraView.\n" +
            "\n" +
            "Control Panel...\n" +
            "\n" +
            "Brings up the Control Panel.\n" +
            "\n" +
            "Help Menu\n" +
            "=========\n" +
            "\n" +
            "Overview\n" +
            "\n" +
            "Displays this message.\n" +
            "\n" +
            "About\n" +
            "\n" +
            "Displays information about the program.\n" +
            "\n" +
            "Control Panel\n" +
            "=============\n" +
            "\n" +
            "MapType\n" +
            "\n" +
            "TerraView gets three types of images: Photo, Topo, and UrbanAreas.\n" +
            "Photo and UrbanAreas are aerial views.  UrbanAreas only exist for\n" +
            "certain parts of the United States but are available in higher\n" +
            "resolution and in color.  The UrbanAreas images are typically newer\n" +
            "than the Photo maps, as well.  Topo images are USGS topological maps.\n" +
            "These are older yet.  If MapPoint 2002 is installed on your system,\n" +
            "TerraView will also get images from MapPoint.  These images show roads\n" +
            "and other landmarks and are a useful addition to the types of data\n" +
            "obtained from the other images.\n" +
            "\n" +
            "Scale\n" +
            "\n" +
            "Allows you to choose the scale for the images.  Scale32m, for example,\n" +
            "means 32 meters per pixel.\n" +
            "\n" +
            "Longitude\n" +
            "Latitude\n" +
            "\n" +
            "Allows you to specify a decimal longitude and latitude.  Longitudes in\n" +
            "the United States are negative.\n" +
            "\n" +
            "Image Width\n" +
            "Image Height\n" +
            "\n" +
            "Allows you to specify the desired image width and height in pixels.\n" +
            "\n" +
            "Pan\n" +
            "\n" +
            "Specifies how far to pan in fractions of the image dimensions.  A\n" +
            "larger number pans further.\n" +
            "\n" +
            "Compass Control\n" +
            "\n" +
            "Allows you to pan and zoom the image.  Panning will be by the fraction\n" +
            "specified in Pan.  Scale will be by one increment in the Scale\n" +
            "drop-down menu.\n" +
            "\n" +
            "Always on top\n" +
            "\n" +
            "Check this to keep the Control Panel above other windows.\n" +
            "\n" +
            "Go Button\n" +
            "\n" +
            "The Go button causes a new image to be obtained.\n" +
            "\n" +
            "Apply Button\n" +
            "\n" +
            "The apply button applies the current values but does not get a new\n" +
            "image.  If you click on the map, it will be re-centered and the type\n" +
            "and scale will be as specified in the Control Panel.  This allows you\n" +
            "to both center and change the type and scale of the image while only\n" +
            "waiting for one download.\n" +
            "\n" +
            "Cancel Button\n" +
            "\n" +
            "Dismisses the Control Panel.\n" +
            "\n";
        #endregion

		public OverviewDlg()
		{
			//
			// Required for Windows Form Designer support
			//
			InitializeComponent();

			//
			// TODO: Add any constructor code after InitializeComponent call
			//
            textBox1.Dock=DockStyle.Fill;
            this.Message=helpText;
            textBox1.SelectionLength=0;

#if false
            richTextBox1.Dock=DockStyle.Fill;
            richTextBox1.LoadFile("..\\..\\help.rtf");
            richTextBox1.Text=helpText;
#endif
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
            System.Resources.ResourceManager resources = new System.Resources.ResourceManager(typeof(OverviewDlg));
            this.textBox1 = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // textBox1
            // 
            this.textBox1.Font = new System.Drawing.Font("Microsoft Sans Serif", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((System.Byte)(0)));
            this.textBox1.Location = new System.Drawing.Point(8, 8);
            this.textBox1.Multiline = true;
            this.textBox1.Name = "textBox1";
            this.textBox1.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.textBox1.Size = new System.Drawing.Size(448, 344);
            this.textBox1.TabIndex = 0;
            this.textBox1.Text = "";
            // 
            // OverviewDlg
            // 
            this.AutoScaleBaseSize = new System.Drawing.Size(5, 13);
            this.ClientSize = new System.Drawing.Size(464, 358);
            this.Controls.Add(this.textBox1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.Name = "OverviewDlg";
            this.Text = "Overview";
            this.ResumeLayout(false);

        }
		#endregion
    
        public String Message {
            get { return String.Join("\n",textBox1.Lines); }
            set { textBox1.Lines=value.Split('\n'); }
        }

    }
}
