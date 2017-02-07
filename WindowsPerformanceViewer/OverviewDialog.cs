﻿using System;
using System.Windows.Forms;

namespace WindowsPerformanceViewer {
    public partial class OverviewDialog : Form {
        /// <summary>
        /// This is the rich text for the overview dialog.  It is best edited
        /// by an application that handles rich text, such as Open Office Write
        /// or Microsoft Word.  It can then be opened in Visual Studio, which
        /// treats it as text.  The contents can then be pasted between the
        /// quotes in the definition.
        /// </summary>
        private static readonly String overviewText =
@"{\rtf1\ansi\deff1\adeflang1025
{\fonttbl{\f0\froman\fprq2\fcharset0 Times New Roman;}{\f1\fswiss\fprq0\fcharset0 Arial;}{\f2\fswiss\fprq2\fcharset0 Arial;}{\f3\fnil\fprq0\fcharset0 Arial;}{\f4\fswiss\fprq0\fcharset0 Arial;}{\f5\fmodern\fprq1\fcharset0 Courier New;}{\f6\fnil\fprq2\fcharset0 SimSun;}{\f7\fnil\fprq2\fcharset0 Microsoft YaHei;}{\f8\fmodern\fprq1\fcharset0 NSimSun;}{\f9\fnil\fprq0\fcharset0 Mangal;}}
{\colortbl;\red0\green0\blue0;\red128\green128\blue128;}
{\stylesheet{\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033\snext1 Normal;}
{\s2\sb240\sa120\keepn\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af2\afs28\lang1081\ltrch\dbch\af7\langfe2052\hich\f2\fs28\lang1033\loch\f2\fs28\lang1033\sbasedon1\snext3 Heading;}
{\s3\sa120\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033\sbasedon1\snext3 Body Text;}
{\s4\sa120\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af9\afs24\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs24\lang1033\loch\f1\fs24\lang1033\sbasedon3\snext4 List;}
{\s5\sb120\sa120\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af9\afs24\lang1081\ai\ltrch\dbch\af6\langfe2052\hich\f3\fs24\lang1033\i\loch\f3\fs24\lang1033\i\sbasedon1\snext5 caption;}
{\s6\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af9\afs24\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs24\lang1033\loch\f1\fs24\lang1033\sbasedon1\snext6 Index;}
{\s7\sb120\sa120\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\afs24\lang1081\ai\ltrch\dbch\af6\langfe2052\hich\fs24\lang1033\i\loch\fs24\lang1033\i\sbasedon1\snext7 caption;}
{\s8\sb120\sa120\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af9\afs24\lang1081\ai\ltrch\dbch\af6\langfe2052\hich\f1\fs24\lang1033\i\loch\f1\fs24\lang1033\i\sbasedon1\snext8 WW-caption;}
{\s9\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af5\afs20\lang1081\ltrch\dbch\af8\langfe2052\hich\f5\fs20\lang1033\loch\f5\fs20\lang1033\sbasedon1\snext9 Preformatted Text;}
}
{\info{\creatim\yr0\mo0\dy0\hr0\min0}{\revtim\yr0\mo0\dy0\hr0\min0}{\printim\yr0\mo0\dy0\hr0\min0}{\comment StarWriter}{\vern3300}}\deftab709
{\*\pgdsctbl
{\pgdsc0\pgdscuse195\pgwsxn12240\pghsxn15840\marglsxn1134\margrsxn1134\margtsxn1134\margbsxn1134\pgdscnxt0 Standard;}}
{\*\pgdscno0}\paperh15840\paperw12240\margl1134\margr1134\margt1134\margb1134\sectd\sbknone\pgwsxn12240\pghsxn15840\marglsxn1134\margrsxn1134\margtsxn1134\margbsxn1134\ftnbj\ftnstart1\ftnrstcont\ftnnar\aenddoc\aftnrstcont\aftnstart1\aftnnrlc
\pard\plain \ltrpar\s1\cf0\qc{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs28\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\fs28\lang1033\b\loch\fs28\lang1033\b {\rtlch \ltrch\loch\f1\fs28\lang1033\i0\b Windows Performance Viewer Overview}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 Windows Performance Viewer accesses the Windows event logs and presents details of the performance in convenient ways.  The logs used in this application are those provided by Microsoft-Windows-Diagnostics-Performance.  You will probably have to run it as 
administrator to get access to the diagnostic logs.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 The application, when it comes up,  gives a short summary table of the boot data for the last 5 boots, along with statistics (min, max, mean, and standard deviation) for all the boots found in the logs.  There are menu items to get more extended informatio
n.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs24\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs24\lang1033\b\loch\f1\fs24\lang1033\b {\rtlch \ltrch\loch\f1\fs24\lang1033\i0\b Boot Performance}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 These data are found in the Applications and Services Logs | Microsoft | Windows | Diagnostics-Performance | Operational for Event ID = 100.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 The {\rtlch\ltrch\dbch\hich\i\loch\i BootTime} is the sum of the {\rtlch\ltrch\dbch\hich\i\loch\i MainPathBootTime,} and the {\rtlch\ltrch\dbch\hich\i\loch\i BootPostBootTime} and is the best figure of merit for the time it takes to boot.  The {\rtlch\ltrch\dbch\hich\i\loch\i MainPathBootTime} is roughly the time it takes for the desktop to become functional.  There is not enough space in 
this overview to describe all the available parameters.  There is more information on the web.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs24\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs24\lang1033\b\loch\f1\fs24\lang1033\b {\rtlch \ltrch\loch\f1\fs24\lang1033\i0\b File Menu}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\b\loch\f1\fs20\lang1033\b {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b Create Boot CSV File}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 Allows you to make a CSV file of all the data and to open it in a spreadsheet.  The latter should work if you have a file association for .csv.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\b\loch\f1\fs20\lang1033\b {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b Boot Plots...}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 Allows you to plot any of the parameters found as a function of the {\rtlch\ltrch\dbch\hich\i\loch\i BootStartTime}, or in fact, any of the other parameters.  The default plot is {\rtlch\ltrch\dbch\hich\i\loch\i BootTime}, {\rtlch\ltrch\dbch\hich\i\loch\i MainPathBootTime}, and {\rtlch\ltrch\dbch\hich\i\loch\i BootPostBootTime} as a function of {\rtlch\ltrch\dbch\hich\i\loch\i BootStartTime}.  The plot view has a list box 
to the left with all the parameters at the top and buttons at the bottom.  You can multi-select items to plot on the y axis and single-select an item to appear on the x axis.  Not all parameters on the x axis give sensible results.  You can however use par
ameters such as {\rtlch\ltrch\dbch\hich\i\loch\i Index}, {\rtlch\ltrch\dbch\hich\i\loch\i BootStartTime}, {\rtlch\ltrch\dbch\hich\i\loch\i BootEndTime}, and {\rtlch\ltrch\dbch\hich\i\loch\i UserBootInstance} meaningfully.  You can also reset the plot to the default and print it.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\b\loch\f1\fs20\lang1033\b {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b Provider Information...}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 Provides information about the Microsoft-Windows-Diagnostics-Performance provider.  This includes the logs and events provided.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\b\loch\f1\fs20\lang1033\b {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b Boot Events...}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 A listing of the boot events in the logs.  These are events with IDs in the 100's.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\b\loch\f1\fs20\lang1033\b {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b Shutdown Events...}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 A listing of the shutdown events in the logs.  These are events with IDs in the 200's.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\b\loch\f1\fs20\lang1033\b {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b Standby Events...}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 A listing of the standby events in the logs.  These are events with IDs in the 300's.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\b\loch\f1\fs20\lang1033\b {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b Monitoring Events...}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 A listing of monitoring events in the logs.  These are events with IDs in the 400's.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\b\loch\f1\fs20\lang1033\b {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b Other Events...}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 A listing of other events in the logs.  These are events with IDs starting with 500 or greater.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\b\loch\f1\fs20\lang1033\b {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b Run Event Viewer}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 Starts the Windows Event Viewer.  This viewer has information about all Windows events, and has more information about performance, in particular, than this application.  The performance events are found under Applications and Services Logs | Microsoft | W
indows | Diagnostics-Performance.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\b\loch\f1\fs20\lang1033\b {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b Exit}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 Exits the application.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs24\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs24\lang1033\b\loch\f1\fs24\lang1033\b {\rtlch \ltrch\loch\f1\fs24\lang1033\i0\b Help Menu}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\b\loch\f1\fs20\lang1033\b {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b Overview}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 This overview.}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ab\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\b\loch\f1\fs20\lang1033\b {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b About}
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\ql\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 
\par \pard\plain \ltrpar\s1\cf0{\*\hyphen2\hyphlead2\hyphtrail2\hyphmax0}\rtlch\af1\afs20\lang1081\ltrch\dbch\af6\langfe2052\hich\f1\fs20\lang1033\loch\f1\fs20\lang1033 {\rtlch \ltrch\loch\f1\fs20\lang1033\i0\b0 About this application.}
\par }";

        public OverviewDialog() {
            InitializeComponent();

            overviewTextBox.Rtf = overviewText;
            // Comes up with all text selected.  Deselect it.
            overviewTextBox.Select(0, 0);
            overviewTextBox.WordWrap = true;
        }
    }
}
