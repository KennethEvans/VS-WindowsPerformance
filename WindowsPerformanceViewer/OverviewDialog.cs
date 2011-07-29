﻿using System;
using System.Windows.Forms;

namespace WindowsPerformanceViewer {
    public partial class OverviewDialog : Form {
        /// <summary>
        /// This is the text for the overview dialog.  It is best edited in a
        /// text editor and copied here.  There are two choices: (1) Leave it
        /// with long lines and set wordwrap on in the TextBox, or (2) Format
        /// it with line endings in the editor and set wordwrap off in the
        /// TextBox.  The second way it is more readable here, but the first
        /// way looks better when it is run.  Set wordwrap in the CTOR, which
        /// will overwrite what is set in the designer.
        /// </summary>
        private static readonly String overviewText =
@"Windows Performance Viewer Overview

Windows Performance Viewer accesses the Windows event logs and presents details of the performance in in a few convenient ways.

Boot Performance

Currently the only performance data presented are for Windows startup.  These data are found in the Applications and Services Logs | Microsoft | Windows | Diagnostics-Performance | Operational for Event ID = 100.  You can use the Windows Event Viewer to view these (and many other) data.  This application, when it comes up,  gives a short summary table of the boot data for the last 5 boots, along with statistics (min, max, mean, and standard deviation) for all of the boots found in the logs.

There is a menu item that allows you to make a CSV file of all of the data and also to open it in a spreadsheet.  The latter should work if you have a file association for .csv.

There is a menu item that allows you to plot any of the parameters found as a function of the BootStartTime, or in fact, any of the other parameters.  The default plot is BootTime, MainPathBootTime, and BootPostBootTime as a function of BootStartTime.  The plot view has a list box to the left with all the parameters at the top and buttons at the bottom.  You can multi-select items to plot on the y axis and single-select an item to appear on the x axis.  Not all parameters on the x axis give sensible results.  You can however use parameters such as Index, BootStartTime, BootEndTime, and UserBootInstance meaningfully.  You can also reset the plot to the default and print it.

The BootTime is the sum of the MainPathBootTime, and the BootPostBootTime and is the best figure of merit for the time it takes to boot.  The MainPathBootTime is roughly the time it takes for the desktop to become functional.  There is not space in this overview to present all the parameters, but there is much information on the web.
";

        public OverviewDialog() {
            InitializeComponent();

            overviewTextBox.Text = overviewText;
            // Comes up with all text selected.  Deselect it.
            overviewTextBox.Select(0, 0);
            overviewTextBox.WordWrap = true;
        }
    }
}
