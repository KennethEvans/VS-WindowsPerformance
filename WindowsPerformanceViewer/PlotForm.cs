using System;
using System.Windows.Forms;
using System.Windows.Forms.DataVisualization.Charting;

namespace WindowsPerformanceViewer {
    public partial class PlotForm : Form {
        private void setYButton_Click(object sender, EventArgs e) {
            String[] selected = getListBoxSelectedNames();
            int nSelected = selected.Length;
            if (selected.Length <= 0) {
                Utils.errMsg("No items selected");
                return;
            }
            String[] newNames = new String[nSelected + 1];
            newNames[0] = colNames[0];
            for(int i = 0; i < nSelected; i++) {
                newNames[i + 1] = selected[i];
            }
            setChartValues(newNames);
        }

        private void setXButton_Click(object sender, EventArgs e) {
            String[] selected = getListBoxSelectedNames();
            int nSelected = selected.Length;
            if (selected.Length <= 0) {
                Utils.errMsg("No items selected");
                return;
            }
            if (selected.Length > 1) {
                Utils.errMsg("More than 1 items selected");
                return;
            }
            // Change the 0 element of the current names
            colNames[0] = (String)selected[0].Clone();
            setChartValues(colNames);
        }

        private void resetButton_Click(object sender, EventArgs e) {
            int len = colNames0.Length;
            String[] newNames = new String[len];
            for (int i = 0; i < len; i++) {
                newNames[i] = (String)colNames0[i].Clone();
            }
            setChartValues(newNames);
        }

        private void printButton_Click(object sender, EventArgs e) {
            PrintingManager printMgr = chart1.Printing;
            printMgr.Print(true);
        }
    }
}
