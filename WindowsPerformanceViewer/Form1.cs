using System;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;

namespace WindowsPerformanceViewer {
    public partial class Form1 : Form {
        public static readonly String LF = "\r\n";
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
    }
}
