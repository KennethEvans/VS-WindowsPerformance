using System;
using System.Collections.Generic;
using System.Data;
using System.Diagnostics;
using System.IO;
using System.Windows.Forms;

namespace WindowsPerformanceViewer {
    public partial class Form1 : Form {
        public static readonly String LF = "\r\n";
        public static readonly String EVENT_VIEWER_CMD =
            @"%windir%\system32\eventvwr.msc /s";
        private static readonly String[] colHeadings = {
            "(Times are in sec)",
            "BootTime",
            "MainPathBootTime",
            "BootPostBootTime",
        };
        private static readonly int nCols = colHeadings.Length;
        private static readonly String[] rowHeadings = {
            "Last",
            "Mean",
            "Standard Deviation",
            "Min",
            "Max",
        };
        private static readonly int nRows = rowHeadings.Length;
        public static readonly double MS2SEC = .001;

        private List<String[]> bootTimes;

        public List<String[]> BootTimes {
            get { return bootTimes; }
            set { bootTimes = value; }
        }

        private String initialDirectory = "";

        /// <summary>
        /// Constructor.
        /// </summary>
        public Form1() {
            InitializeComponent();
            // Create the DataSource
            DataTable table = getResultsTable();
            if (table == null) {
                MessageBox.Show("Unable to get data.  You may have to run as administrator.",
                    "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            dataGridView1.DataSource = getResultsTable();
            dataGridView1.Columns[0].DefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleLeft;
            //dataGridView1.Columns[1].DefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleCenter;
            //dataGridView1.Columns[2].DefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleCenter;
            //dataGridView1.Columns[3].DefaultCellStyle.Alignment = DataGridViewContentAlignment.MiddleCenter;
        }

        /// <summary>
        /// Builds a DataTable of the boot times data.
        /// </summary>
        public DataTable getResultsTable() {
            bootTimes = DiagnosticsUtils.getBootTimes();
            DiagnosticsUtils.setStatistics(bootTimes);
            if (bootTimes == null) {
                // TODO
                return null;
            }
            int count = bootTimes.Count;
            // Do the last nLast data points
            int nLast = 5;
            // The last item before the statistics is count -5
            int lastPos = count - 4 - nLast;
            if (lastPos <= 0) {
                // TODO
                return null;
            }

            String[][] data = bootTimes.ToArray();
            int nItems = data[0].Length;

            // Convert all the times to sec
            double doubleVal;
            for (int j = 0; j < count; j++) {
                for (int i = 1; i < nCols; i++) {
                    try {
                        doubleVal = Convert.ToDouble(data[j][i]);
                    } catch (FormatException) {
                        doubleVal = Double.NaN;
                    } catch (OverflowException) {
                        doubleVal = Double.NaN;
                    }
                    doubleVal *= MS2SEC;
                    data[j][i] = String.Format("{0:0.0}", doubleVal);
                }
            }

            // Create the output table.
            DataTable dt = new DataTable();

            // Add the columns
            foreach (string heading in colHeadings) {
                dt.Columns.Add(heading, typeof(String));
            }

            // Add the rows
            for (int i = lastPos; i < count; i++) {
                dt.Rows.Add(data[i][0], data[i][1], data[i][2], data[i][3]);
            }
            return dt;
        }

        /// <summary>
        /// Common method used to display event summaries.
        /// </summary>
        /// <param name="startId">Starting event ID.</param>
        /// <param name="endId">Ending eventId.</param>
        private void showSummaryDialog(String title, int startId, int endId) {
            InformationDialog dlg = new InformationDialog();
            Cursor oldCursor = Cursor.Current;
            Cursor.Current = Cursors.WaitCursor;
            String info = title + LF;
            info += "Data as of: " + DateTime.Now + LF + LF;
            info += DiagnosticsUtils.getDiagnosticsLogForEvents(startId, endId);
            dlg.Text = title;
            dlg.TextBox.WordWrap = true;
            dlg.TextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            dlg.ClientSize = new System.Drawing.Size(500, 500);


            if (info != null) {
                dlg.TextBox.Text = info;
            } else {
                dlg.TextBox.Text = "Unable to get " + title;
            }
            // Unselect the text just added
            dlg.TextBox.Select(0, 0);

            Cursor.Current = oldCursor;
            dlg.Show();
        }

        /////////////////////////////////////////////////////////////////////
        // Event Handlers 
        /////////////////////////////////////////////////////////////////////

        private void exitToolStripMenuItem_Click(object sender, EventArgs e) {
            Application.Exit();
        }

        private void createCSVFileMenuItem_Click(object sender, EventArgs e) {
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

        private void plotMenuItem_Click(object sender, EventArgs e) {
            Cursor oldCursor = Cursor.Current;
            Cursor.Current = Cursors.WaitCursor;
            PlotForm plotForm = new PlotForm();
            plotForm.StartPosition = FormStartPosition.CenterParent;

            Cursor.Current = oldCursor;
            plotForm.Show();
        }

        private void aboutMenuItem_Click(object sender, EventArgs e) {
            AboutBox dlg = new AboutBox();
            dlg.ShowDialog();
        }

        private void overviewMenuItem_Click(object sender, EventArgs e) {
            OverviewDialog dlg = new OverviewDialog();
            dlg.Show();
        }

        private void eventDescriptionMenuItem_Click(object sender, EventArgs e) {
            InformationDialog dlg = new InformationDialog();
            Cursor oldCursor = Cursor.Current;
            Cursor.Current = Cursors.WaitCursor;
            String info = DiagnosticsUtils.getDiagnosticsEventProvider();
            dlg.Text = "Provider Information";
            dlg.TextBox.WordWrap = true;
            dlg.TextBox.ScrollBars = System.Windows.Forms.ScrollBars.Both;

            if (info != null) {
                dlg.TextBox.Text = info;
            } else {
                dlg.TextBox.Text = "Unable to get event description information";
            }
            // Unselect the text just added
            dlg.TextBox.Select(0, 0);

            Cursor.Current = oldCursor;
            dlg.Show();
        }

        private void runEventViewerMenuItem_Click(object sender, EventArgs e) {
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

        private void bootEventsMenuItem_Click(object sender, EventArgs e) {
            showSummaryDialog("Boot Events", 100, 199);
        }

        private void shutdownEventsMenuItem_Click(object sender, EventArgs e) {
            showSummaryDialog("Shutdown Events", 200, 299);
        }

        private void standbyEventsMenuItem_Click(object sender, EventArgs e) {
            showSummaryDialog("Standby Events", 300, 399);
        }

        private void monitoringEventsMenuItem_Click(object sender, EventArgs e) {
            showSummaryDialog("Performance Monitoring Events", 400, 499);
        }

        private void otherEventsMenuItem_Click(object sender, EventArgs e) {
            showSummaryDialog("Other Events", 500, 10000);
        }
    }
}
