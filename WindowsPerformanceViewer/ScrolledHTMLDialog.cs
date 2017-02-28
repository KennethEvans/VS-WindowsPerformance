﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace WindowsPerformanceViewer {
    public partial class ScrolledHTMLDialog : Form {
        public ScrolledHTMLDialog() {
            InitializeComponent();

            // Add the HTML
            string appDir = System.IO.Path.GetDirectoryName(
                System.Reflection.Assembly.GetExecutingAssembly().GetName().CodeBase);
            webBrowser.Url = new Uri(System.IO.Path.Combine(appDir, @"Help\Overview.html"));

        }

        private void OnFormClosing(object sender, FormClosingEventArgs e) {
            // Just hide rather than close if the user did it
            if (e.CloseReason == CloseReason.UserClosing) {
                e.Cancel = true;
                Visible = false;
            }
        }
    }
}
