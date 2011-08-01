using System;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;
using System.Security.Principal;

namespace WindowsPerformanceViewer {
    public partial class Form1 : Form {
        public static readonly String LF = "\r\n";
        public static readonly String EVENT_VIEWER_CMD =
            @"%windir%\system32\eventvwr.msc /s";
        private String initialDirectory = "";

        private void exitToolStripMenuItem_Click(object sender, EventArgs e) {
            Application.Exit();
        }

        private void createCSVFileToolStripMenuItem_Click(object sender, EventArgs e) {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Filter =
               "csv files (*.csv)|*.csv|All files (*.*)|*.*";
            dialog.InitialDirectory = initialDirectory;
            dialog.Title = "Select a CSV file to Write";
            dialog.CheckFileExists = false;
            DialogResult res = dialog.ShowDialog();
            if (res != DialogResult.OK) {
                return;
            }
            String fileName = dialog.FileName;
            if (fileName == null) {
                return;
            }
            initialDirectory = dialog.InitialDirectory;
            if (File.Exists(fileName)) {
                res = MessageBox.Show("File exists: " + fileName + LF + "OK to overwrite?",
                    fileName, MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                if (res != DialogResult.Yes) {
                    return;
                }
            }
            Cursor oldCursor = Cursor.Current;
            Cursor.Current = Cursors.WaitCursor;
            String resString = DiagnosticsUtils.createBootTimesCsvFile(fileName);
            Cursor.Current = oldCursor;
            if (resString.Equals("OK")) {
                MessageBox.Show("Created: " + fileName + LF +
                    "Trying to start a spreadsheet with it.", "Info",
                    MessageBoxButtons.OK, MessageBoxIcon.Information);
                Process.Start(fileName);
            } else {
                MessageBox.Show(resString, "Error",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void plotToolStripMenuItem_Click(object sender, EventArgs e) {
            Cursor oldCursor = Cursor.Current;
            Cursor.Current = Cursors.WaitCursor;
            PlotForm plotForm = new PlotForm();
            plotForm.StartPosition = FormStartPosition.CenterParent;
            plotForm.Show();
            Cursor.Current = oldCursor;
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e) {
            AboutBox dlg = new AboutBox();
            dlg.ShowDialog();
        }

        private void overviewToolStripMenuItem_Click(object sender, EventArgs e) {
            OverviewDialog dlg = new OverviewDialog();
            dlg.ShowDialog();
        }

        private void eventDescriptionToolStripMenuItem_Click(object sender, EventArgs e) {
            InformationDialog dlg = new InformationDialog();
            String info = DiagnosticsUtils.getDiagnosticsEventProvider();
            dlg.Text = "Provider Information";
            dlg.TextBox.WordWrap = true;
            dlg.TextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;

            if (info != null) {
                dlg.TextBox.Text = info;
            } else {
                dlg.TextBox.Text = "Unable to get eve nt provider information";
            }
            // Unselect the text just added
            dlg.TextBox.Select(0, 0);

            dlg.ShowDialog();
        }

        private void runEventViewerToolStripMenuItem_Click(object sender, EventArgs e) {
#if false
            WindowsPrincipal principal = new WindowsPrincipal(WindowsIdentity.GetCurrent());
            bool hasAdministrativeRight = principal.IsInRole(WindowsBuiltInRole.Administrator);
            if (!hasAdministrativeRight) {
                DialogResult res =
                    MessageBox.Show("This application requires admin privilages."
                    + LF + "Click Ok to elevate or Cancel to exit.",
                    "Elevate?",
                    MessageBoxButtons.OKCancel,
                    MessageBoxIcon.Question);
                if (res == DialogResult.OK) {
                    // TODO
                } else {
                    // TODO
                }
            }
#endif
            try {
                ProcessStartInfo startInfo = new ProcessStartInfo();
                startInfo.Verb = "runas";
                startInfo.FileName = "eventvwr.msc";
                startInfo.Arguments = "/s";
                Process.Start(startInfo);
            } catch (Exception ex) {
                Utils.excMsg("Problems starting the Event Viewer", ex);
            }
        }
    }
}
