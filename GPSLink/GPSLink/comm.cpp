// Communications routines for GPSLink

#include "stdafx.h"

#define DEBUG_PORT 0
#define DEBUG_BYTES 0
#define DEBUG_READ 0
#define DEBUG_INVALID_DATA 0
#define DEBUG_CRC 0
#define DEBUG_CHECKSUM 0
#define DEBUG_WAYPOINT 0
#define DEBUG_HANG 0
#define DEBUG_ACKNAK 0
#define DEBUG_UPLOAD 0
#define HANDLE_IEEE 0
#define DEBUG_SELECT 0
#define PRINT_DATA_RCV 0
#define DEBUG_RECV 0

// Doesn't seem to fix no progress dialog problem
#define USE_CRITICAL_SECTION 0
// Does seem to fix no progress dialog problem
#define USE_RECV_KLUDGE 1

#define HEX 1
#define BYTES_PER_LINE 10

#define ANSI_STRINGSIZE 52
#define TIME_STRINGSIZE 30
#define DLE 16
#define ETX 3

#define PVT_COUNT 5

#include "resource.h"
#include <math.h>
#include "GPSLink.h"

typedef enum {
	Pkt_None,
	Pkt_Start,
	Pkt_Size,
	Pkt_Data,
	Pkt_Checksum,
	Pkt_End
} ReadPacketState;

typedef enum {
	CIP_None,
	CIP_Abort,
	CIP_WptDownload,
	CIP_WptUpload,
	CIP_RteDownload,
	CIP_RteUpload,
	CIP_TrkDownload,
	CIP_TrkUpload,
} CommandInProgress;

typedef enum {
	CS_None,
	CS_Started,
	CS_RecordsRcvd,
	CS_HeaderSent,
	CS_HeaderRcvd,
	CS_PointSent,
	CS_PointRcvd,
	CS_AckRcvd,
	CS_NakRcvd,
	CS_TransferComplete,
} CommandState;

// Function prototypes
static BOOL showCommError(void);
static BOOL PortWrite(BYTE);
static BOOL readPacket(BYTE *data, BYTE *buf, PacketType *pktType,
					   BYTE *nData, DWORD *nBuf);
static BOOL cleanupPacket(BYTE *pBuf, DWORD *nBuf);
static void sendAck(PacketType pktType);
static void sendNak(PacketType pktType);
static void sendRecords(WORD nPackets);
static void sendTransferComplete(CommandType cmdType);
static BOOL sendCommand(CommandType cmdType);
static void startPVTData(void);
static void trafficController(void);
static WORD calculateWptPackets(void);
static WORD calculateRtePackets(void);
static WORD calculateTrkPackets(void);

// Global variables
CPosition position;
CBlockList waypointList;
CBlockList routeList;
CBlockList trackList;
CBlockList clipboardList;
BOOL pvtDataInProgress=FALSE;
static BOOL restartPvtData=FALSE;
static WORD nPackets=0;
static WORD nPacketsTrans=0;
static CTrack *currentTrack=NULL;
static CRoute *currentRoute=NULL;
static CWaypoint *currentWaypoint=NULL;
static CommandType commandRequested=Cmnd_None;
static PacketType ackNakType=Pid_Unknown;
static CommandInProgress commandInProgress=CIP_None;
static CommandState commandState=CS_None;
static WORD cmdType;

static BOOL showCommError(void)
{
	BOOL status;
	COMSTAT comStat;
	DWORD   dwErrors;
	DWORD   dwError;

	// Get and clear current errors on the port.
	status=ClearCommError(hPort,&dwErrors,&comStat);
	if(!status && hPort != INVALID_HANDLE_VALUE) {
		dwError=GetLastError();
		errMsg(_T("ClearCommError failed [%d]"),dwError);
		return FALSE;
	}

	// Get error flags
#if 0
	// Parallel port errors on W95/98 
	dwErrors & CE_DNS;
	dwErrors & CE_PTO;
	dwErrors & CE_OOP;
#endif
	if(dwErrors & CE_IOE) {
		lbprintf(_T("I/O error!"));
	}
	if(dwErrors & CE_MODE) {
		lbprintf(_T("Invalid handle or mode!"));
	}
	if(dwErrors & CE_BREAK) {
		lbprintf(_T("Break condition!"));
	}
	if(dwErrors & CE_FRAME) {
		lbprintf(_T("Frame error!"));
	}
	if(dwErrors & CE_RXOVER) {
		lbprintf(_T("Input buffer overflow!"));
	}
	if(dwErrors & CE_TXFULL) {
		lbprintf(_T("Output buffer full!"));
	}
	if(dwErrors & CE_OVERRUN) {
		lbprintf(_T("Buffer overrun (lost character)!"));
	}
	if(dwErrors & CE_RXPARITY) {
		lbprintf(_T("Parity error!"));
	}

	// COMSTAT structure contains info about communications status
	if(comStat.fCtsHold) {
		// Tx waiting for CTS signal
		lbprintf(_T("Tx waiting for CTS signal"));
	}
	if(comStat.fDsrHold) {
		// Tx waiting for DSR signal
		lbprintf(_T("Tx waiting for DSR signal"));
	}
	if(comStat.fRlsdHold) {
		// Tx waiting for RLSD signal
		lbprintf(_T("Tx waiting for RLSD signal"));
	}
	if(comStat.fXoffHold) {
		// Tx waiting, XOFF char rec'd
		lbprintf(_T("Tx waiting, XOFF char rec'd"));
	}
	if(comStat.fXoffSent) {
		// Tx waiting, XOFF char sent
		lbprintf(_T("Tx waiting, XOFF char sent"));
	}
	if(comStat.fEof) {
		// EOF character received
		lbprintf(_T("EOF character received"));
	}
	if(comStat.fTxim) {
		// Character waiting for Tx; char queued with TransmitCommChar
		lbprintf(_T("Character waiting for Tx"));
	}
	if(comStat.cbInQue) {
		// comStat.cbInQue bytes have been received, but not read
		lbprintf(_T("%d bytes received, but not read"),comStat.cbInQue);
	}
	if(comStat.cbOutQue) {
		// comStat.cbOutQue bytes are waiting
		lbprintf(_T("%d bytes waiting to be sent"),comStat.cbOutQue);
	}

	if(!dwErrors) return TRUE;
	else return FALSE;
}

BOOL PortClose(void)
{
	BOOL status;
	DWORD dwError;

	if(hPort != INVALID_HANDLE_VALUE) {
		// Close the communication port
		status=CloseHandle(hPort);
		if(!status) {
			dwError = GetLastError();
			errMsg(_T("Error closing port [%d]"),dwError);
			return FALSE;
		} else {
			hPort = INVALID_HANDLE_VALUE;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL PortInitialize(LPTSTR lpszPortName)
{
	BOOL status;
	DWORD dwError;
	DWORD dwThreadID;
	DCB PortDCB;
	COMMTIMEOUTS CommTimeouts;

#if DEBUG_PORT
	// Set the error to zero since we will be checking errors when there
	// may not have been one
	SetLastError(0);
#endif

	// Print the time offset
#if 0
	lbprintf(_T("GMT Offset=%ld hr"),getTimeOffset()/3600);
#endif

	// Open the serial port
	hPort = CreateFile(lpszPortName, // Pointer to the name of the port
		GENERIC_READ | GENERIC_WRITE,
		// Access (read-write) mode
		0,            // Share mode
		NULL,         // Pointer to the security attribute
		OPEN_EXISTING,// How to open the serial port
		0,            // Port attributes
		NULL);        // Handle to port with attribute
	// to copy

	// If it fails to open the port, return FALSE
	if(hPort == INVALID_HANDLE_VALUE) {
		// Could not open the port
		dwError = GetLastError();
		errMsg(_T("Unable to connect to %s [%d]"),lpszPortName,dwError);
		return FALSE;
	}
#if DEBUG_PORT
	dwError = GetLastError();
	infoMsg(_T("Last error is %d after opening port, hPort=%x (%s)"),
		dwError,hPort,
		hPort==INVALID_HANDLE_VALUE?_T("Invalid"):_T("Valid"));
	SetLastError(0);
#endif
	PortDCB.DCBlength = sizeof(DCB);     

	// Get the default port setting information
	status=GetCommState(hPort,&PortDCB);
	if(!status) {
		dwError = GetLastError();
		errMsg(_T("Unable to get communication parameters from port[%d]"),
			dwError);
		return FALSE;
	}
#if DEBUG_PORT
	dwError = GetLastError();
	infoMsg(_T("Last error is %d after GetCommState"),
		dwError);
	SetLastError(0);
#endif

	// Change the DCB structure settings.  Cannot use any flow control
	// except XON/XOFF because there aren't enough wires for those signals.
	PortDCB.fOutxCtsFlow = FALSE;         // No CTS output flow control 
	PortDCB.fOutxDsrFlow = FALSE;         // No DSR output flow control 
	PortDCB.fRtsControl = RTS_CONTROL_DISABLE;  // RTS flow control type 
	PortDCB.fDtrControl = DTR_CONTROL_DISABLE;  // DTR flow control type 
	PortDCB.fTXContinueOnXoff = TRUE;     // XOFF continues Tx 
	PortDCB.fOutX = FALSE;                 // XON/XOFF out flow control 
	PortDCB.fInX = FALSE;                  // XON/XOFF in flow control 

	PortDCB.fDsrSensitivity = FALSE;      // DSR sensitivity 
	PortDCB.fBinary = TRUE;               // Binary mode; no EOF check 
	PortDCB.fParity = FALSE;              // Enable parity checking 
	PortDCB.fErrorChar = FALSE;           // Disable error replacement 
	PortDCB.fNull = FALSE;                // Disable null stripping 
	PortDCB.fAbortOnError = FALSE;        // Do not abort reads/writes on  error

	PortDCB.BaudRate = CBR_9600;              // Baud for Garmin
	PortDCB.ByteSize = 8;                 // Number of bits/byte, 4-8 
	PortDCB.Parity = NOPARITY;            // 0-4=no,odd,even,mark,space 
	PortDCB.StopBits = ONESTOPBIT;        // 0,1,2 = 1, 1.5, 2 

	// Configure the port according to the specifications of the DCB
	// structure
	status=SetCommState(hPort, &PortDCB);
	if(!status) {
		// Could not configure the port
		dwError = GetLastError();
		errMsg(_T("Unable to set communication parameters [%d]"),dwError);
		return FALSE;
	}
#if DEBUG_PORT
	dwError = GetLastError();
	infoMsg(_T("Last error is %d after SetCommState"),
		dwError);
	SetLastError(0);
#endif

	// Retrieve the time-out parameters for all read and write
	// operations on the port
	status=GetCommTimeouts(hPort, &CommTimeouts);
	if(!status) {
		dwError = GetLastError();
		errMsg(_T("Unable to get timeout parameters from port[%d]"),
			dwError);
	}

	// Change the COMMTIMEOUTS structure settings.  Cannot use
	// ReadIntervalTimeout alone, since it is measured from the first
	// character and will wait forever if no character is received
	CommTimeouts.ReadIntervalTimeout = 0;  
	CommTimeouts.ReadTotalTimeoutMultiplier = 10;  
	CommTimeouts.ReadTotalTimeoutConstant = 1000;    
	CommTimeouts.WriteTotalTimeoutMultiplier = 10;  
	CommTimeouts.WriteTotalTimeoutConstant = 1000;    

	// Set the time-out parameters for all read and write operations
	// on the port
	status=SetCommTimeouts(hPort, &CommTimeouts);
	if(!status) {
		// Could not set timeout parameters
		dwError = GetLastError();
		errMsg(_T("Unable to set timeout parameters [%d]"),dwError);
		return FALSE;
	} 
#if DEBUG_PORT
	dwError = GetLastError();
	infoMsg(_T("Last error is %d after SetCommTimeouts"),
		dwError);
	SetLastError(0);
#endif

#if 0
	// Direct the port to perform extended functions SETDTR and SETRTS
	// SETDTR: Sends the DTR (data-terminal-ready) signal
	// SETRTS: Sends the RTS (request-to-send) signal
	EscapeCommFunction(hPort,SETDTR);
	EscapeCommFunction(hPort,SETRTS);
#endif

	// Create a read thread for reading data from the communication port
	if(hReadThread=CreateThread(NULL,0,PortReadThread,NULL,0,&dwThreadID)) {
		CloseHandle(hReadThread);
	} else {
		// Could not create the read thread
		dwError = GetLastError();
		errMsg(_T("Unable to create read thread [%d]"),dwError);
		return FALSE;
	}

	return TRUE;
}

static BOOL PortWrite(BYTE Byte)
{
	DWORD dwError;
	DWORD dwNumBytesWritten;

	if(!WriteFile(
		hPort,              // Port handle
		&Byte,              // Pointer to the data to write 
		1,                  // Number of bytes to write
		&dwNumBytesWritten, // Pointer to the number of bytes written
		NULL)               // Must be NULL for Windows CE
		) {              
			// WriteFile failed. Report error.
			dwError = GetLastError();
			PlaySound(_T("Critical"),NULL,SND_SYNC);
			return FALSE;
	}

	return TRUE;
}

DWORD WINAPI PortReadThread(LPVOID lpvoid)
{
	DWORD dwError;
	BOOL status,status1;
	BYTE nData=0;
	DWORD nBuf=0;
	BYTE data[255];
	BYTE buf[DATA_BUFSIZE];
	PacketType pktType=Pid_Unknown;
	WCHAR *unicodeString=NULL;
	BOOL restart;

#if 0
	// Set the priority below normal to prevent degrading of other
	// performance when position data is coming in
	SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL);
#endif

	// KE: IN WIN32 WaitCommEvent is not terminated by
	// CloseHandle(hPort).  Instead, CloseHandle hangs, at least when no
	// characters are coming in.  Therefore, don't use WaitCommEvent but
	// rely on timeouts from ReadFile.

#if DEBUG_HANG > 1
	lbprintf(_T("PortReadThread entered %s"),
		(hPort == INVALID_HANDLE_VALUE)?_T("(hPort Invalid)"):_T(""));
#endif
	while(hPort != INVALID_HANDLE_VALUE) {
#if 0
		BYTE byte=0;
		DWORD dwBytesTransferred=0;

		status=ReadFile(hPort,&byte,1,&dwBytesTransferred,NULL);
		lbprintf(_T("Rcvd %d bytes [%d]"),dwBytesTransferred,byte);
#endif	
		status=readPacket(data,buf,&pktType,&nData,&nBuf);
		dwError=GetLastError();
#if DEBUG_HANG
		status1=showCommError();
#endif
#if DEBUG_READ
		lbprintf(_T("%s Pkt %d: %d bytes %d data last=%d"),
			(!status && GetLastError()==ERROR_TIMEOUT )?_T("TMO"):_T("RCV"),
			pktType,nBuf,nData,nBuf?buf[nBuf-1]:0);
#endif
		if(!status) {
			switch(dwError) {
		case ERROR_INVALID_HANDLE:
			return(0);
		case ERROR_CRC:
			// Checksum failed
			status1=cleanupPacket(buf+nBuf,&nBuf);
			if(!status1) {
				lbprintf(_T("cleanupPacket failed"));
			} else {
				lbprintf(_T("cleanupPacket succeeded"));
			}
#if DEBUG_CRC
			dumpBytes(buf,nBuf);
#endif
#if 0
			// NAK this packet
			if(pktType != Pid_Unknown) sendNak(pktType);
			continue;
#else
			// ACK this packet temporarily
			if(pktType != Pid_Unknown) sendAck(pktType);
#endif
			break;
		case ERROR_INVALID_DATA:
			lbprintf(_T("readPacket failed:\n")
				_T("Type=%d nData=%d nBuf=%d"),
				pktType,nData,nBuf);
#if DEBUG_INVALID_DATA
			dumpBytes(buf,nBuf);
#endif
			// NAK this packet
			if(pktType != Pid_Unknown) sendNak(pktType);
			continue;
		case ERROR_TIMEOUT:
		default:
			// Give up the time slice
			Sleep(0);
			continue;
			}
		}

		// Branch on packet type
#if USE_CRITICAL_SECTION
		CRITICAL_SECTION cs;
		InitializeCriticalSection(&cs);
		EnterCriticalSection(&cs);
#endif
		switch(pktType) {
	case Pid_Position_Data:
		lbprintf(_T("Position initialization rcvd [%d bytes]"),nData);
		break;
	case Pid_Records:
#if PRINT_DATA_RCV
		lbprintf(_T("Records data rcvd [%d bytes]"),nData);
#endif
		memcpy(&nPackets,data,2);
#if USE_RECV_KLUDGE
		// Without this statement, it doesn't always get this case
		// for the PPC
		lbprintf(_T("  Initializing transfer of %d records"),nPackets);
#endif
		commandState=CS_RecordsRcvd;
		trafficController();
		break;
	case Pid_Xfer_Cmplt:
#if PRINT_DATA_RCV
		lbprintf(_T("Transfer complete rcvd [%d bytes]"),nData);
#endif
		memcpy(&cmdType,data,2);
		restart=FALSE;
		commandState=CS_TransferComplete;
		trafficController();
		break;
	case Pid_Wpt_Data:
		{
			CWaypoint *waypoint;

			nPacketsTrans++;
			progressSetProgress(_T("Waypoints"),nPacketsTrans,nPackets);
#if PRINT_DATA_RCV
			lbprintf(_T("Waypoint data rcvd [%d bytes]"),nData);
#endif
#if DEBUG_WAYPOINT
			lbprintf(_T("  nData=%02x [%d] nBuf=%02x [%d]"),
				nData,nData,nBuf,nBuf);
			dumpBytes(buf,nBuf);
#endif
			waypoint=new CWaypoint(FALSE);
			if(!waypoint) {
				lbprintf(_T("Cannot create waypoint"));
				break;
			}
			status=waypoint->setData(data,nData);
			if(!status) {
				lbprintf(_T("Cannot set waypoint data"));
				break;
			}
#if PRINT_DATA_RCV && 0
			waypoint->writeLB();
#endif
			if(waypoint->isDuplicate(&waypointList)) {
				delete waypoint;
			} else {
				waypointList.addBlock((CBlock *)waypoint);
			}
			break;
		}
	case Pid_Rte_Hdr:
		{
			nPacketsTrans++;
			progressSetProgress(_T("Routes"),nPacketsTrans,nPackets);
#if PRINT_DATA_RCV
			lbprintf(_T("Route header rcvd [%d bytes]"),nData);
#endif
			currentRoute=new CRoute(FALSE);
			if(!currentRoute) {
				lbprintf(_T("Cannot create route"));
				break;
			}
			routeList.addBlock((CBlock *)currentRoute);
			status=currentRoute->setData(data,nData);
			if(!status) {
				lbprintf(_T("Cannot set route header data"));
				break;
			}
#if PRINT_DATA_RCV && 0
			currentRoute->writeLB();
#endif
			break;
		}
	case Pid_Rte_Wpt_Data:
		{
			CWaypoint *waypoint;

			nPacketsTrans++;
			progressSetProgress(_T("Routes"),nPacketsTrans,nPackets);
#if PRINT_DATA_RCV
			lbprintf(_T("Route waypoint data rcvd [%d bytes]"),nData);
#endif
#if 0
			dumpBytes(buf,nBuf);
#endif
			if(!currentRoute) {
				lbprintf(_T("Current route is not valid"));
				break;
			}
			waypoint=new CWaypoint(TRUE);
			if(!waypoint) {
				lbprintf(_T("Cannot create route waypoint"));
				break;
			}
			currentRoute->addBlock((CBlock *)waypoint);
			currentWaypoint=waypoint;
			status=waypoint->setData(data,nData);
			if(!status) {
				lbprintf(_T("Cannot set waypoint data"));
				break;
			}
#if PRINT_DATA_RCV
			listWaypoint(waypoint);
#endif
			break;
		}
	case Pid_Rte_Link_Data:
		{
			nPacketsTrans++;
			progressSetProgress(_T("Routes"),nPacketsTrans,nPackets);
#if PRINT_DATA_RCV
			lbprintf(_T("Route link data rcvd [%d bytes]"),nData);
#endif
#if 0
			dumpBytes(buf,nBuf);
#endif
			if(!currentWaypoint) {
				lbprintf(_T("Current waypoint is not valid"));
				break;
			}
			status=currentWaypoint->setLinkData(data,nData);
			if(!status) {
				lbprintf(_T("Cannot set route link data"));
				break;
			}
#if PRINT_DATA_RCV
			listWaypoint(currentWaypoint);
#endif
			break;
		}
	case Pid_Trk_Hdr:
		{
			nPacketsTrans++;
			progressSetProgress(_T("Tracks"),nPacketsTrans,nPackets);
#if PRINT_DATA_RCV
			lbprintf(_T("Track header rcvd [%d bytes]"),nData);
#endif
			currentTrack=new CTrack(FALSE);
			if(!currentTrack) {
				lbprintf(_T("Cannot create track"));
				break;
			}
			trackList.addBlock((CBlock *)currentTrack);
			status=currentTrack->setData(data,nData);
			if(!status) {
				lbprintf(_T("Cannot set track header data"));
				break;
			}
#if PRINT_DATA_RCV && 0
			currentTrack->writeLB();
#endif
			break;
		}
	case Pid_Trk_Data:
		{
			CTrackpoint *trackpoint;

			nPacketsTrans++;
			progressSetProgress(_T("Tracks"),nPacketsTrans,nPackets);
#if PRINT_DATA_RCV
			lbprintf(_T("Trackpoint data rcvd [%d bytes]"),nData);
#endif
			if(!currentTrack) {
				lbprintf(_T("Current track is not valid"));
				break;
			}
			trackpoint=new CTrackpoint(TRUE);
			if(!trackpoint) {
				lbprintf(_T("Cannot create trackpoint"));
				break;
			}
			currentTrack->addBlock((CBlock *)trackpoint);
			status=trackpoint->setData(data,nData);
			if(!status) {
				lbprintf(_T("Cannot set trackpoint data"));
				break;
			}
#if PRINT_DATA_RCV && 0
			trackpoint->writeLB();
#endif
			break;
		}
	case Pid_Product_Data:
		lbprintf(_T("Product data rcvd [%d bytes]"),nData);
		lbprintf(_T("Product ID=%d Version=%.2f"),
			(int)data[0],.01*(double)(int)data[2]);
		{
			char *desc=(char *)data+4;
			LPWSTR unicodeString=NULL;

			ansiToUnicode(desc,&unicodeString);
			if(unicodeString) {
				lbprintf(unicodeString);
				infoMsg(_T("%s\nProduct ID %d"),
					unicodeString,(int)data[0]);
				free(unicodeString);
			}
		}
		break;
	case Pid_Protocol_Array:
		lbprintf(_T("Protocol capability rcvd [%d bytes]"),nData);
		break;
	case Pid_Pvt_Data:
		{
			status=position.setData(data,nData);
			if(!status) {
				lbprintf(_T("Cannot set position data"));
				break;
			}
			// Redraw current position
#ifdef UNDER_CE
			// The screen update is slow on WCE
			// May want to do this anyway
			static int count=PVT_COUNT-1;

			count++;
			if(count > PVT_COUNT) count=1;
			if(count != PVT_COUNT) break;
#endif
			if(hMap) SendMessage(hMap,WM_DRAWPOS,0,0);
			lbclear();
			lbprintf(_T("PVT data rcvd [%d bytes]"),nData);
			listPosition(&position);
			break;
		}
	case Pid_Ack_Byte:
		ackNakType=(PacketType)data[0];
#if DEBUG_ACKNAK || DEBUG_UPLOAD
		lbprintf(_T("ACK rcvd [%d]"),ackNakType);
#endif
#if DEBUG_BYTES
		dumpBytes(buf,nBuf);
#endif
		switch(commandInProgress) {
	case CIP_WptUpload:
	case CIP_RteUpload:
	case CIP_TrkUpload:
		commandState=CS_AckRcvd;
		trafficController();
		break;
		}
		continue;
	case Pid_Nak_Byte:
		ackNakType=(PacketType)data[0];
		lbprintf(_T("NAK rcvd [%d]"),ackNakType);
#if DEBUG_BYTES
		dumpBytes(buf,nBuf);
#endif
		switch(commandInProgress) {
	case CIP_WptUpload:
	case CIP_RteUpload:
	case CIP_TrkUpload:
		commandState=CS_NakRcvd;
		trafficController();
		break;
		}
		continue;
	default:
		lbprintf(_T("Type %d packet rcvd [%d bytes]"),pktType,nData);
#if 1
		dumpBytes(buf,nBuf);
#endif
		break;
		}
#if USE_CRITICAL_SECTION
		LeaveCriticalSection(&cs);
#endif
		// ACK this packet
		sendAck(pktType);
	}

	return (0);
}

// Reads a packet, returns TRUE on success, FALSE on failure
// Sets last error on failure to one of
// ERROR_INVALID_HANDLE, ERROR_TIMEOUT, ERROR_INVALID_DATA
static BOOL readPacket(BYTE *data, BYTE *buf, PacketType *pktType,
					   BYTE *nData, DWORD *nBuf)
{
	DWORD dwError;
	BOOL status;
	BYTE byte;
	BYTE *pData=data;
	BYTE *pBuf=buf;
	DWORD dwBytesTransferred;
	ReadPacketState readState=Pkt_None;
	unsigned long checksum=0;
	BOOL dlePending=FALSE;
	DWORD nDataRead;


	*nData=0;
	*nBuf=nDataRead=0;
	*pktType=Pid_Unknown;
	while(TRUE) {
		if(hPort == INVALID_HANDLE_VALUE) {
			SetLastError(ERROR_INVALID_HANDLE);
			return FALSE;
		}
		status=ReadFile(hPort,&byte,1,&dwBytesTransferred,NULL);
		if(!status) {
			dwError = GetLastError();
			// Do an error message except from closing the port handle
			if(dwError != ERROR_INVALID_HANDLE) {
				errMsg(_T("Error reading port [%d]"),dwError);
				return FALSE;
			}
		}
		if(!dwBytesTransferred) {
			SetLastError(ERROR_TIMEOUT);
			return FALSE;
		}

		// Process byte
		switch(readState) {
	case Pkt_None:
		dlePending=FALSE;
		*pBuf++=byte; (*nBuf)++;
		if(byte == DLE) {
			readState=Pkt_Start;
		} else {
			// Not a packet start
			*nBuf=0;
			pBuf=buf;
			continue; // Try again (skip quit)
		}
		break;
	case Pkt_Start:
		*pBuf++=byte; (*nBuf)++;
		if(byte == ETX || byte == DLE) {
			// Cannot be a packet ID
			// May be a 2nd DLE or a packet end
			readState=Pkt_None;
			*nBuf=0;
			pBuf=buf;
			continue; // Try again (skip quit)
		}
		*pktType=(PacketType)byte;
		checksum+=byte;
		readState=Pkt_Size;
		break;
	case Pkt_Size:
		*pBuf++=byte; (*nBuf)++;
		// Handle DLE stuffing
		if(byte == DLE) {
			if(dlePending) {
				if(byte == ETX) {
					// Abnormal end of packet
					lbprintf(_T("Abnormal end of packet"));
					SetLastError(ERROR_INVALID_DATA);
					return FALSE;
				}
				if(byte != DLE) {
					// Should not happen
					lbprintf(_T("Bad data (single DLE)"));
					SetLastError(ERROR_INVALID_DATA);
					return FALSE;
				}
			} else {
				// Is a possible first DLE of a DLE DLE sequence
				dlePending=TRUE;
				break;
			}
		}
		dlePending=FALSE;
		*nData=byte;
		checksum+=byte;
		if(*nData) readState=Pkt_Data;
		else readState=Pkt_Checksum;
		break;
	case Pkt_Data:
		*pBuf++=byte; (*nBuf)++;
		// Handle DLE stuffing
		if(byte == DLE) {
			if(dlePending) {
				if(byte == ETX) {
					// Abnormal end of packet
					lbprintf(_T("Abnormal end of packet"));
					SetLastError(ERROR_INVALID_DATA);
					return FALSE;
				}
				if(byte != DLE) {
					// Should not happen
					lbprintf(_T("Bad data (single DLE)"));
					SetLastError(ERROR_INVALID_DATA);
					return FALSE;
				}
			} else {
				// Is a possible first DLE of a DLE DLE sequence
				dlePending=TRUE;
				break;
			}
		}
		// Store byte
		dlePending=FALSE;
		*pData++=byte; nDataRead++;
		checksum+=byte;
		if(nDataRead == (DWORD)(*nData)) {
			readState=Pkt_Checksum;
		}
		break;
	case Pkt_Checksum:
		*pBuf++=byte; (*nBuf)++;
		// Handle DLE stuffing
		if(byte == DLE) {
			if(dlePending) {
				if(byte == ETX) {
					// Abnormal end of packet
					lbprintf(_T("Abnormal end of packet"));
					SetLastError(ERROR_INVALID_DATA);
					return FALSE;
				}
				if(byte != DLE) {
					// Should not happen
					lbprintf(_T("Bad data (single DLE)"));
					SetLastError(ERROR_INVALID_DATA);
					return FALSE;
				}
			} else {
				// Is a possible first DLE of a DLE DLE sequence
				dlePending=TRUE;
				break;
			}
		}
		dlePending=FALSE;
		checksum=(~checksum+1)&0xFF;
		if((BYTE)checksum != byte) {
			PlaySound(_T("Critical"),NULL,SND_SYNC);
			lbprintf(_T("\nBad checksum: Calc: %02x Rcvd: %02x\n"),
				checksum,byte);
			SetLastError(ERROR_CRC);
			return FALSE;
		}
		readState=Pkt_End;
		break;
	case Pkt_End:
		*pBuf++=byte; (*nBuf)++;
		if(byte == DLE) {
			if(dlePending) {
				lbprintf(_T("Expected ETX not found"));
				SetLastError(ERROR_INVALID_DATA);
				return FALSE;
			} else {
				// Found expected DLE
				dlePending=TRUE;
			}
		} else if(byte == ETX) {
			if(dlePending) {
				// Normal end of packet
				readState=Pkt_None;
			} else {
				// Never got DLE
				lbprintf(_T("Got ETX, expected DLE"));
				SetLastError(ERROR_INVALID_DATA);
				return FALSE;
			}
		} else {
			// Unexpected character
			lbprintf(_T("Invalid end of packet: %02x"),byte);
			SetLastError(ERROR_INVALID_DATA);
			return FALSE;
		}
		break;
	default:
		lbprintf(_T("Invalid read sequence: State=%d"),readState);
		SetLastError(ERROR_INVALID_DATA);
		return FALSE;
		}
		// Quit when the read state is Pkt_None
		if(readState == Pkt_None) break;
	}

	return TRUE;
}

// Make a type packet in buf using nData bytes in data, return
// length bytes written
BOOL sendPacket(BYTE *data, BYTE *buf, PacketType pktType,
				BYTE nData, DWORD *nBuf)
{
	BYTE *pBuf=buf;
	BOOL status,retVal=TRUE;;
	DWORD i;
	unsigned long checksum;
	BYTE checkbyte;

	// Make the packet
	*nBuf=0;
	*pBuf++=DLE; (*nBuf)++;
	*pBuf++=pktType; (*nBuf)++;
	*pBuf++=nData; (*nBuf)++;
	if(nData == DLE) {*pBuf++=DLE; (*nBuf)++;}
	checksum=pktType+nData;
	for(i=0; i < nData; i++) {
		*pBuf++=*data; (*nBuf)++;
		if(*data == DLE) {*pBuf++=DLE; (*nBuf)++;}
		checksum+=*data++;
	}
	checkbyte=(BYTE)((~checksum+1)&0x00ff);
	*pBuf++=checkbyte; (*nBuf)++;
	if(checkbyte == DLE) {*pBuf++=DLE; (*nBuf)++;}
	*pBuf++=DLE; (*nBuf)++;
	*pBuf++=ETX; (*nBuf)++;

#if 0
	// Suspend the read thread so we don't read and write at the same
	// time
	do {
		DWORD count=SuspendThread(hReadThread);
	} while(count == 0xffffffff);
#endif

	// Send the packet
	for(i=0; i < *nBuf; i++) {
		status=PortWrite(buf[i]);
		if(!status) {
			lbprintf(_T("sendPacket: Write error at %d of %d bytes"),
				i+1,*nBuf);
			retVal=FALSE;
			break;
		}
	}
#if DEBUG_HANG > 1
	lbprintf(_T("%d bytes written for pkt %d"),*nBuf,pktType);
#endif

#if 0
	// Resume the thread
	status=ResumeThread(hReadThread);
	if(status == 0xffffffff) {
		dwError=GetLastError();
		errMsg(_T("ResumeThread failed [%d].\nDisconnect."),dwError);
	}
	if(status > 1) {
		errMsg(_T("Thread is still suspended.\nDisconnect."));
	}
#endif
	return TRUE;
}

static void sendAck(PacketType pktType)
{
	BOOL status;
	DWORD nBuf=0;
	BYTE nData=1;
	BYTE data[1];
	BYTE buf[MAXPACKETSIZE(1)];   // Allow for DLE stuffing

	data[0]=pktType;
	status=sendPacket(data,buf,Pid_Ack_Byte,nData,&nBuf);
	if(!status) {
		lbprintf(_T("Error sending ACK"));
	}
#if DEBUG_ACKNAK
	lbprintf(_T("ACK sent"));
#endif
#if DEBUG_BYTES
	dumpBytes(buf,nBuf);
#endif
}

static void sendNak(PacketType pktType)
{
	BOOL status;
	DWORD nBuf=0;
	BYTE nData=1;
	BYTE data[1];
	BYTE buf[MAXPACKETSIZE(1)];   // Allow for DLE stuffing

	data[0]=pktType;
	status=sendPacket(data,buf,Pid_Nak_Byte,nData,&nBuf);
	if(!status) {
		lbprintf(_T("Error sending NAK"));
	}
#if DEBUG_ACKNAK
	lbprintf(_T("NAK sent"));
#endif
#if DEBUG_BYTES
	dumpBytes(buf,nBuf);
#endif
}

static void sendRecords(WORD nPackets)
{
	BOOL status;
	DWORD nBuf=0;
	BYTE nData=2;
	BYTE data[2];
	BYTE buf[MAXPACKETSIZE(2)];   // Allow for DLE stuffing

	CPY2(data,&nPackets);
	status=sendPacket(data,buf,Pid_Records,nData,&nBuf);
	if(!status) {
		lbprintf(_T("Error sending Records packet"));
	}
#if DEBUG_UPLOAD
	lbprintf(_T("Records packet [%d] [%d packets] sent"),
		Pid_Records,nPackets);
#endif
#if DEBUG_BYTES
	dumpBytes(buf,nBuf);
#endif
}

static void sendTransferComplete(CommandType cmdType)
{
	BOOL status;
	DWORD nBuf=0;
	BYTE nData=1;
	BYTE data[1];
	BYTE buf[MAXPACKETSIZE(1)];   // Allow for DLE stuffing

	data[0]=cmdType;
	status=sendPacket(data,buf,Pid_Xfer_Cmplt,nData,&nBuf);
	if(!status) {
		lbprintf(_T("Error sending Transfer Complete"));
	}
#if DEBUG_UPLOAD
	lbprintf(_T("Transfer Complete [%d] [Type %d] sent"),
		Pid_Xfer_Cmplt,cmdType);
#endif
#if DEBUG_BYTES
	dumpBytes(buf,nBuf);
#endif
}

BOOL GetID(void)
{
	BOOL status;
	unsigned int checksum=0;
	BOOL dlePending=0;
	DWORD nBuf=0;
	BYTE nData;
	BYTE data[255];
	BYTE buf[DATA_BUFSIZE];
	BYTE *pData=data;
	PacketType pktType;

	// Send Pid_ProductRequest
	lbprintf(_T("Sending product data request"));
	pktType=Pid_Product_Rqst;
	nData=0;
	status=sendPacket(data,buf,pktType,nData,&nBuf);
	if(!status) {
		lbprintf(_T("Error sending product data request"));
	}

#if DEBUG_BYTES
	dumpBytes(buf,nBuf);
#endif

	return TRUE;
}

//Read until no characters or DLE-ETX.  Return TRUE only if DLE-ETX
static BOOL cleanupPacket(BYTE *pBuf, DWORD *nBuf)
{
	BOOL status;
	BYTE byte;
	DWORD dwBytesTransferred;
	DWORD dwError;
	BOOL dlePending=FALSE;

	while(TRUE) {
		if(hPort == INVALID_HANDLE_VALUE) {
			SetLastError(ERROR_INVALID_HANDLE);
			return FALSE;
		}
		status=ReadFile(hPort,&byte,1,&dwBytesTransferred,NULL);
		if(!status) {
			dwError = GetLastError();
			// Do an error message except from closing the port handle
			if(dwError != ERROR_INVALID_HANDLE) {
				errMsg(_T("Error reading port [%d]"),dwError);
				return FALSE;
			}
		}
		if(!dwBytesTransferred) {
			return FALSE;
		}
		*pBuf++=byte; (*nBuf)++;
		if(dlePending) {
			if(byte == ETX) {
				// Normal end of packet
				return TRUE;
			}
			if(byte != DLE) {
				// Should not happen
				dlePending=FALSE;
				continue;
			} else {
				// 2nd DLE
				dlePending=FALSE;
				continue;
			}
		} else {
			if(byte == DLE) {
				dlePending=TRUE;
				continue;
			} else {
				// Normal character
				continue;
			}
		}
	}
}

void queueCommand(CommandType cmdType)
{
#if USE_CRITICAL_SECTION
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	EnterCriticalSection(&cs);
#endif
	commandRequested=cmdType;
	trafficController();
#if USE_CRITICAL_SECTION
	LeaveCriticalSection(&cs);
#endif
}

void abortCurrentTransfer(void)
{
#if USE_CRITICAL_SECTION
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	EnterCriticalSection(&cs);
#endif
	// Clear the command queue
	commandInProgress=CIP_Abort;
	trafficController();

	// Send the abort command
	queueCommand(Cmnd_Abort_Transfer);
	progressDestroyDialog();
	trafficController();
#if USE_CRITICAL_SECTION
	LeaveCriticalSection(&cs);
#endif
}

static BOOL sendCommand(CommandType cmdType)
{
	BOOL checkPVT=FALSE; 
	BOOL commandNecessary=TRUE;
	BOOL status;
	DWORD nBuf=0;
	BYTE nData=2;
	BYTE data[2];
	BYTE buf[11];     // 6 + 1 + (2 + 2)=4 possible DLE

#if USE_CRITICAL_SECTION
	CRITICAL_SECTION cs;
	InitializeCriticalSection(&cs);
	EnterCriticalSection(&cs);
#endif

#if DEBUG_RECV && 0
	lbprintf(_T("sendCommand(1): CR: %d CIP: %d CS: %d"),
		commandRequested,commandInProgress,commandState);
#endif
	// Set before switches
	switch(cmdType) {
	case Cmnd_Transfer_Rte:
	case Cmnd_Transfer_Trk:
	case Cmnd_Transfer_Wpt:
		checkPVT=TRUE;
		break;
	case Cmnd_Upload_Rte:
	case Cmnd_Upload_Trk:
	case Cmnd_Upload_Wpt:
		checkPVT=TRUE;
		commandNecessary=FALSE;
		break;
	case Cmnd_Start_Pvt_Data:
	case Cmnd_Stop_Pvt_Data:
		break;
	case Cmnd_Abort_Transfer:
		break;
	case Cmnd_Transfer_Alm:
	case Cmnd_Transfer_Posn:
	case Cmnd_Transfer_Prx:
	case Cmnd_Transfer_Time:
	case Cmnd_Turn_Off_Pwr:
		errMsg(_T("Command not implemented %d"),cmdType);
		return FALSE;
	default:
		errMsg(_T("Invalid command %d"),cmdType);
		return FALSE;
	}

	// Stop PVT data
	if(checkPVT && pvtDataInProgress) {
		status=sendCommand(Cmnd_Stop_Pvt_Data);
		if(!status) {
			errMsg(_T("Could not stop PVT data for command %d "),cmdType);
			return FALSE;
		}
		restartPvtData=TRUE;
	}

	// Send the packet (There are no packets sent to initiate an upload)
	if(commandNecessary) {
		data[0]=cmdType;
		data[1]=0;
		status=sendPacket(data,buf,Pid_Command_Data,nData,&nBuf);
		if(!status) {
			lbprintf(_T("Error sending command %d"),cmdType);
			return FALSE;
		}
		lbprintf(_T("Command %d sent"),cmdType);
	} else {
		lbprintf(_T("Command %d started"),cmdType);
	}

	// Set after switches
	switch(cmdType) {
	case Cmnd_Transfer_Rte:
		commandInProgress=CIP_RteDownload;
		commandState=CS_Started;
		break;
	case Cmnd_Transfer_Trk:
		commandInProgress=CIP_TrkDownload;
		commandState=CS_Started;
		break;
	case Cmnd_Transfer_Wpt:
		commandInProgress=CIP_WptDownload;
		commandState=CS_Started;
		break;
	case Cmnd_Upload_Rte:
		commandInProgress=CIP_RteUpload;
		commandState=CS_Started;
		break;
	case Cmnd_Upload_Trk:
		commandInProgress=CIP_TrkUpload;
		commandState=CS_Started;
		break;
	case Cmnd_Upload_Wpt:
		commandInProgress=CIP_WptUpload;
		commandState=CS_Started;
		break;
	case Cmnd_Start_Pvt_Data:
		// Draw the current position using stored values
		pvtDataInProgress=TRUE;  // Must preceed draw
		if(hMap) SendMessage(hMap,WM_BEGINDRAWPOS,0,0);
		break;
	case Cmnd_Stop_Pvt_Data:
		// Draw the current position using stored values
		if(pvtDataInProgress) {
			if(hMap) SendMessage(hMap,WM_ENDDRAWPOS,0,0);
			pvtDataInProgress=FALSE;  // Must follow draw
		}
		break;
	case Cmnd_Abort_Transfer:
		// Needs to be fully implemented
		if(restartPvtData) {
			status=sendCommand(Cmnd_Start_Pvt_Data);
			if(!status) {
				return FALSE;
			}
			restartPvtData=FALSE;
		}
	case Cmnd_Transfer_Alm:
	case Cmnd_Transfer_Posn:
	case Cmnd_Transfer_Prx:
	case Cmnd_Transfer_Time:
	case Cmnd_Turn_Off_Pwr:
		// Not implemented
		break;
	default:
		break;
	}
#if DEBUG_BYTES
	dumpBytes(buf,nBuf);
#endif

#if DEBUG_RECV
	lbprintf(_T("sendCommand(2): CR: %d CIP: %d CS: %d"),
		commandRequested,commandInProgress,commandState);
#endif
#if USE_CRITICAL_SECTION
	LeaveCriticalSection(&cs);
#endif

	return TRUE;
}

static void startPVTData(void)
{
	BOOL status;

	status=sendCommand(Cmnd_Start_Pvt_Data);
	if(!status) {
		lbprintf(_T("Could not restart PVT data"));
	} else {
		restartPvtData=FALSE;
	}
}

static WORD calculateWptPackets(void)
{
	WORD nPackets=0;

	// Set the current selections into the blocks
	wptGetSelections();

	tsDLIterBD<CBlock> iter(waypointList.first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		CBlock *pBlock=iter;
		if(pBlock->isChecked()) {
			nPackets++;
#if DEBUG_SELECT
			{
				LPWSTR unicodeString=NULL;

				ansiToUnicode(pBlock->getIdent(),&unicodeString);
				if(unicodeString) {
					lbprintf(_T("Checked: %s"),unicodeString);
					free(unicodeString);
				} else {
					lbprintf(_T("Checked: <Unknown>"));
				}
			}
#endif	    
		}
		++iter;
	}

#if DEBUG_SELECT
	lbprintf(_T("Total packets=%d"),nPackets);
#endif

	return nPackets;
}

static WORD calculateRtePackets(void)
{
	WORD nPackets=0;

	// Set the current selections into the blocks
	rteGetSelections();

	tsDLIterBD<CBlock> iter(routeList.first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		CBlock *pBlock=iter;
		if(pBlock->isChecked()) {
			nPackets++;
#if DEBUG_SELECT
			{
				LPWSTR unicodeString=NULL;

				ansiToUnicode(pBlock->getIdent(),&unicodeString);
				if(unicodeString) {
					lbprintf(_T("Checked: %s"),unicodeString);
					free(unicodeString);
				} else {
					lbprintf(_T("Checked: <Unknown>"));
				}
			}
#endif
			// Do the waypoints
			const CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock=iter1;
				if(pBlock->isChecked()) {
					nPackets+=2;
#if DEBUG_SELECT
					{
						LPWSTR unicodeString=NULL;

						ansiToUnicode(pBlock->getIdent(),&unicodeString);
						if(unicodeString) {
							lbprintf(_T("Checked: %s"),unicodeString);
							free(unicodeString);
						} else {
							lbprintf(_T("Checked: <Unknown>"));
						}
					}
#endif
				}
				iter1++;
			}
		}
		iter++;
	}

	return nPackets;
}

static WORD calculateTrkPackets(void)
{
	WORD nPackets=0;

	// Set the current selections into the blocks
	trkGetSelections();

	tsDLIterBD<CBlock> iter(trackList.first());
	tsDLIterBD<CBlock> eol;
	while(iter != eol) {
		CBlock *pBlock=iter;
		if(pBlock->isChecked()) {
			nPackets++;
#if DEBUG_SELECT
			{
				LPWSTR unicodeString=NULL;

				ansiToUnicode(pBlock->getIdent(),&unicodeString);
				if(unicodeString) {
					lbprintf(_T("Checked: %s"),unicodeString);
					free(unicodeString);
				} else {
					lbprintf(_T("Checked: <Unknown>"));
				}
			}
			WORD nTracks=0;
#endif
			// Do the trackpoints
			const CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
			tsDLIterBD<CBlock> iter1(blockList->first());
			while(iter1 != eol) {
				CBlock *pBlock=iter1;
				if(pBlock->isChecked()) {
					nPackets++;
#if DEBUG_SELECT
					nTracks++;
#endif
				}
				iter1++;
			}
#if DEBUG_SELECT
			lbprintf(_T("Checked: %d tracks"),nTracks);
#endif
		}
		iter++;
	}

	return nPackets;
}

static void trafficController(void)
{
	static tsDLList<CCommand> commandQueue;
	static tsDLIterBD<CBlock> iter;
	static tsDLIterBD<CBlock> iter1;
	static tsDLIterBD<CBlock> eol;
	CBlock *pBlock=NULL,*pBlock1=NULL;
	BOOL status;

#if DEBUG_RECV
	lbprintf(_T("trafficController: CR: %d CIP: %d CS: %d"),
		commandRequested,commandInProgress,commandState);
#endif

	// Check for abort
	if(commandInProgress == CIP_Abort) {
		// Clear the command queue
		while(commandQueue.count()) {
			CCommand *cmd=commandQueue.first();
			commandQueue.remove(*cmd);
			delete cmd;
		}
		// Reset the counts
		currentWaypoint=NULL;
		currentRoute=NULL;
		currentTrack=NULL;
		nPackets=0;
		nPacketsTrans=0;
		commandState=CS_None;
		commandInProgress=CIP_None;

		return;
	}


	// If there is a requested command, queue it
	if(commandRequested != Cmnd_None) {
		CCommand *cmd=new CCommand(commandRequested);
		commandRequested=Cmnd_None;
		if(!cmd) {
			errMsg(_T("Cannot queue command %d"),commandRequested);
			return;
		}
		commandQueue.add(*cmd);
	}

	// Check if there is a command in progress
	if(commandInProgress == CIP_None) {
		// There is none, check the queue
		if(commandQueue.count()) {
			// There is a queued command.  Send it.
			CCommand *cmd=commandQueue.first();
			commandQueue.remove(*cmd);
			sendCommand(cmd->getCommand());
			delete cmd;
		} else {
			// No queued command.  Nothing to do.
			return;
		}
	}

	// There is a command in progress
	switch(commandInProgress) {
	case CIP_WptDownload:
		switch(commandState) {
	case CS_None:
		break;
	case CS_Started:
		break;
	case CS_RecordsRcvd:
		lbprintf(_T("  Downloading %d waypoint packets"),nPackets);
		progressCreateDialog(hInst,hWndMain);
		nPacketsTrans=0;
		progressSetProgress(_T("Waypoints"),nPacketsTrans,nPackets);
		break;
	case CS_HeaderSent:
		break;
	case CS_HeaderRcvd:
		break;
	case CS_PointSent:
		break;
	case CS_PointRcvd:
		break;
	case CS_AckRcvd:
		break;
	case CS_NakRcvd:
		break;
	case CS_TransferComplete:
		lbprintf(_T("  Rcvd %d of %d waypoint packets"),
			nPacketsTrans,nPackets);
		nPackets=nPacketsTrans=0;
		progressDestroyDialog();
		commandState=CS_None;
		commandInProgress=CIP_None;
		// Redisplay the waypoints if visible
		if(hWptLV) wptCreateLV(hWndMain);
		// Repaint the map if it exists
		if(hMap) {
			InvalidateRect(hMap,NULL,FALSE);
			UpdateWindow(hMap);
		}
		// Restart the PVT data
		if(restartPvtData) startPVTData();
		break;
		}
		break;
	case CIP_WptUpload:
		switch(commandState) {
	case CS_None:
		break;
	case CS_Started:
		// Send Pid_Records
		nPackets=calculateWptPackets();
		if(!waypointList.getCount() || !nPackets) {
			// Nothing to send
			PlaySound(_T("Critical"),NULL,SND_SYNC);
			commandState=CS_None;
			commandInProgress=CIP_None;
		} else {
			lbprintf(_T("  Uploading %d waypoint packets"),nPackets);
			commandState=CS_HeaderSent;
			progressCreateDialog(hInst,hWndMain);
			nPacketsTrans=0;
			progressSetProgress(_T("Waypoints"),nPacketsTrans,nPackets);
			sendRecords(nPackets);
		}
		break;
	case CS_RecordsRcvd:
		break;
	case CS_HeaderSent:
		break;
	case CS_HeaderRcvd:
		break;
	case CS_PointSent:
		break;
	case CS_PointRcvd:
		break;
	case CS_AckRcvd:
		switch(ackNakType) {
	case Pid_Records:
		// Send first checked waypoint
		iter=waypointList.first();
		while(iter != eol) {
			pBlock=iter;
			if(pBlock->isChecked()) break;
			iter++;
		}
		if(iter == eol) {
			// End of waypoints
			sendTransferComplete(Cmnd_Transfer_Wpt);
			break;
		}
		pBlock=iter++;
		nPacketsTrans++;
		status=pBlock->sendData(Pid_Wpt_Data);
		if(!status) {
			lbprintf(_T("Error sending waypoint packet %d"),
				nPacketsTrans);
		}
#if DEBUG_UPLOAD || DEBUG_SELECT
		{
			LPWSTR unicodeString=NULL;

			ansiToUnicode(pBlock->getIdent(),&unicodeString);
			if(unicodeString) {
				lbprintf(_T("Sent waypoint [%d] %s"),
					Pid_Wpt_Data,unicodeString);
				free(unicodeString);
			} else {
				lbprintf(_T("Sent waypoint [%d] %s"),
					Pid_Wpt_Data,_T("<Unknown>"));
			}
		}
#endif
		progressSetProgress(_T("Waypoints"),nPacketsTrans,nPackets);
		break;
	case Pid_Xfer_Cmplt:
		lbprintf(_T("  Sent %d of %d waypoint packets"),
			nPacketsTrans,nPackets);
		progressDestroyDialog();
		commandState=CS_None;
		commandInProgress=CIP_None;
		// Restart the PVT data
		if(restartPvtData) startPVTData();
		break;
	case Pid_Wpt_Data:
		// Send next checked waypoint
		while(iter != eol) {
			pBlock=iter;
			if(pBlock->isChecked()) break;
			iter++;
		}
		if(iter == eol) {
			// End of waypoints
			sendTransferComplete(Cmnd_Transfer_Wpt);
			break;
		}
		pBlock=iter++;
		nPacketsTrans++;
		status=pBlock->sendData(Pid_Wpt_Data);
		if(!status) {
			lbprintf(_T("Error sending waypoint packet %d"),
				nPacketsTrans);
		}
#if DEBUG_UPLOAD || DEBUG_SELECT
		{
			LPWSTR unicodeString=NULL;

			ansiToUnicode(pBlock->getIdent(),&unicodeString);
			if(unicodeString) {
				lbprintf(_T("Sent waypoint [%d] %s"),
					Pid_Wpt_Data,unicodeString);
				free(unicodeString);
			} else {
				lbprintf(_T("Sent waypoint [%d] %s"),
					Pid_Wpt_Data,_T("<Unknown>"));
			}
		}
#endif
		progressSetProgress(_T("Waypoints"),nPacketsTrans,nPackets);
		break;
		}
		break;
	case CS_NakRcvd:
		progressDestroyDialog();
		errMsg(_T("Communication error. Waypoint transfer aborted."));
		sendTransferComplete(Cmnd_Transfer_Wpt);
		commandState=CS_None;
		commandInProgress=CIP_None;
		// Restart the PVT data
		if(restartPvtData) startPVTData();
		break;
	case CS_TransferComplete:
		break;
		}
		break;
	case CIP_RteDownload:
		switch(commandState) {
	case CS_None:
		break;
	case CS_Started:
		break;
	case CS_RecordsRcvd:
		lbprintf(_T("  Downloading %d route packets"),nPackets);
		progressCreateDialog(hInst,hWndMain);
		nPacketsTrans=0;
		progressSetProgress(_T("Routes"),nPacketsTrans,nPackets);
		break;
	case CS_HeaderSent:
		break;
	case CS_HeaderRcvd:
		break;
	case CS_PointSent:
		break;
	case CS_PointRcvd:
		break;
	case CS_AckRcvd:
		break;
	case CS_NakRcvd:
		break;
	case CS_TransferComplete:
		lbprintf(_T("  Rcvd %d of %d route packets"),
			nPacketsTrans,nPackets);
		nPackets=nPacketsTrans=0;
		currentTrack=NULL;
		progressDestroyDialog();
		commandState=CS_None;
		commandInProgress=CIP_None;
		// Redisplay the routes if visible
		if(hRteLV) rteCreateLV(hWndMain);
		// Repaint the map if it exists
		if(hMap) {
			InvalidateRect(hMap,NULL,FALSE);
			UpdateWindow(hMap);
		}
		// Restart the PVT data
		if(restartPvtData) startPVTData();
		break;
		}
		break;
	case CIP_RteUpload:
		switch(commandState) {
	case CS_None:
		break;
	case CS_Started:
		// Send Pid_Records
		nPackets=calculateRtePackets();
		if(!routeList.getCount() || !nPackets) {
			// Nothing to send
			PlaySound(_T("Critical"),NULL,SND_SYNC);
			commandState=CS_None;
			commandInProgress=CIP_None;
		} else {
			lbprintf(_T("  Uploading %d route packets"),nPackets);
			commandState=CS_HeaderSent;
			progressCreateDialog(hInst,hWndMain);
			nPacketsTrans=0;
			progressSetProgress(_T("Routes"),nPacketsTrans,nPackets);
			sendRecords(nPackets);
		}
		break;
	case CS_RecordsRcvd:
		break;
	case CS_HeaderSent:
		break;
	case CS_HeaderRcvd:
		break;
	case CS_PointSent:
		break;
	case CS_PointRcvd:
		break;
	case CS_AckRcvd:
		switch(ackNakType) {
	case Pid_Records:
		// Send first checked route header
		iter=routeList.first();
		while(iter != eol) {
			pBlock=iter;
			if(pBlock->isChecked()) break;
			iter++;
		}
		if(iter == eol) {
			// End of routes
			sendTransferComplete(Cmnd_Transfer_Rte);
			break;
		}
		pBlock=iter;  // Don't increment
		nPacketsTrans++;
		status=pBlock->sendData(Pid_Rte_Hdr);
		if(!status) {
			lbprintf(_T("Error sending route header packet %d"),
				nPacketsTrans);
		}
#if DEBUG_UPLOAD || DEBUG_SELECT
		{
			LPWSTR unicodeString=NULL;

			ansiToUnicode(pBlock->getIdent(),&unicodeString);
			if(unicodeString) {
				lbprintf(_T("Sent route header [%d] %s"),
					Pid_Rte_Hdr,unicodeString);
				free(unicodeString);
			} else {
				lbprintf(_T("Sent route header [%d] %s"),
					Pid_Rte_Hdr,_T("<Unknown>"));
			}
		}
#endif
		progressSetProgress(_T("Routes"),nPacketsTrans,nPackets);
		break;
	case Pid_Xfer_Cmplt:
		lbprintf(_T("  Sent %d of %d route packets"),
			nPacketsTrans,nPackets);
		progressDestroyDialog();
		commandState=CS_None;
		commandInProgress=CIP_None;
		// Restart the PVT data
		if(restartPvtData) startPVTData();
		break;
	case Pid_Rte_Hdr:
	case Pid_Rte_Link_Data:
		while(iter != eol) {
			pBlock=iter;
			CBlockList *blockList=((CRoute *)pBlock)->getBlockList();
			// If a route header was just sent start at the
			// beginning, otherwise use the existing iter1
			if(ackNakType == Pid_Rte_Hdr) iter1=blockList->first();
			// Look for the first checked waypoint
			while(iter1 != eol) {
				pBlock1=iter1;
				if(pBlock1->isChecked()) {
					// Send the waypoint
					nPacketsTrans++;
					status=pBlock1->sendData(Pid_Rte_Wpt_Data);
					if(!status) {
						lbprintf(_T("Error sending route waypoint packet %d"),
							nPacketsTrans);
					}
#if DEBUG_UPLOAD
					{
						LPWSTR unicodeString=NULL;

						ansiToUnicode(pBlock1->getIdent(),&unicodeString);
						if(unicodeString) {
							lbprintf(_T("Sent route waypoint [%d] %s"),
								Pid_Rte_Wpt_Data,unicodeString);
							free(unicodeString);
						} else {
							lbprintf(_T("Sent route waypoint [%d] %s"),
								Pid_Rte_Wpt_Data,_T("<Unknown>"));
						}
					}
#endif
					progressSetProgress(_T("Routes"),nPacketsTrans,nPackets);
					break;
				}
				iter1++;
			}
			// Check if we sent something, and break if so
			if(iter1 != eol) {
				iter1;  // Don't increment here because link is next
				break;
			}
			iter++;
			// Check if we have sent all routes
			if(iter == eol) break;
			// Send the next route header
			pBlock=iter;
			if(pBlock->isChecked()) {
				// Send the header
				nPacketsTrans++;
				status=pBlock->sendData(Pid_Rte_Hdr);
				if(!status) {
					lbprintf(_T("Error sending route header packet %d"),
						nPacketsTrans);
				}
#if DEBUG_UPLOAD
				{
					LPWSTR unicodeString=NULL;

					ansiToUnicode(pBlock->getIdent(),&unicodeString);
					if(unicodeString) {
						lbprintf(_T("Sent route header [%d] %s"),
							Pid_Rte_Hdr,unicodeString);
						free(unicodeString);
					} else {
						lbprintf(_T("Sent route header [%d] %s"),
							Pid_Rte_Hdr,_T("<Unknown>"));
					}
				}
#endif
				progressSetProgress(_T("Routes"),
					nPacketsTrans,nPackets);
				break;
			}
		}  // End while over routes
		// Check if we are done with the routes
		if(iter == eol) {
			sendTransferComplete(Cmnd_Transfer_Rte);
		}
		break;
	case Pid_Rte_Wpt_Data:
		// We don't have to check.  If there was waypoint there
		// will be an associated link.  Send associated link data.
		pBlock=iter1++;
		nPacketsTrans++;
		status=pBlock->sendData(Pid_Rte_Link_Data);
		if(!status) {
			lbprintf(_T("Error sending route waypoint link packet %d"),
				nPacketsTrans);
		}
#if DEBUG_UPLOAD
		{
			LPWSTR unicodeString=NULL;

			ansiToUnicode(pBlock->getIdent(),&unicodeString);
			if(unicodeString) {
				lbprintf(_T("Sent route waypoint link [%d] %s"),
					Pid_Rte_Link_Data,unicodeString);
				free(unicodeString);
			} else {
				lbprintf(_T("Sent route waypoint link [%d] %s"),
					Pid_Rte_Link_Data,_T("<Unknown>"));
			}
		}
#endif
		progressSetProgress(_T("Routes"),nPacketsTrans,nPackets);
		break;
		}  // End switch on ackNakType
		break;
	case CS_NakRcvd:
		break;
	case CS_TransferComplete:
		break;
		}
		break;
	case CIP_TrkDownload:
		switch(commandState) {
	case CS_None:
		break;
	case CS_Started:
		break;
	case CS_RecordsRcvd:
		lbprintf(_T("  Downloading %d track packets"),nPackets);
		progressCreateDialog(hInst,hWndMain);
		nPacketsTrans=0;
		progressSetProgress(_T("Tracks"),nPacketsTrans,nPackets);
		break;
	case CS_HeaderSent:
		break;
	case CS_HeaderRcvd:
		break;
	case CS_PointSent:
		break;
	case CS_PointRcvd:
		break;
	case CS_AckRcvd:
		break;
	case CS_NakRcvd:
		break;
	case CS_TransferComplete:
		lbprintf(_T("  Rcvd %d of %d track packets"),
			nPacketsTrans,nPackets);
		nPackets=nPacketsTrans=0;
		currentTrack=NULL;
		progressDestroyDialog();
		commandState=CS_None;
		commandInProgress=CIP_None;
		// Redisplay the tracks if visible
		if(hTrkLV) trkCreateLV(hWndMain);
		// Repaint the map if it exists
		if(hMap) {
			InvalidateRect(hMap,NULL,FALSE);
			UpdateWindow(hMap);
		}
		// Restart the PVT data
		if(restartPvtData) startPVTData();
		break;
		}
		break;
	case CIP_TrkUpload:
		switch(commandState) {
	case CS_None:
		break;
	case CS_Started:
		// Send Pid_Records
		nPackets=calculateTrkPackets();
		if(!trackList.getCount() || !nPackets) {
			// Nothing to send
			PlaySound(_T("Critical"),NULL,SND_SYNC);
			commandState=CS_None;
			commandInProgress=CIP_None;
		} else {
			lbprintf(_T("  Uploading %d track packets"),nPackets);
			commandState=CS_HeaderSent;
			progressCreateDialog(hInst,hWndMain);
			nPacketsTrans=0;
			progressSetProgress(_T("Tracks"),nPacketsTrans,nPackets);
			sendRecords(nPackets);
		}
		break;
	case CS_RecordsRcvd:
		break;
	case CS_HeaderSent:
		break;
	case CS_HeaderRcvd:
		break;
	case CS_PointSent:
		break;
	case CS_PointRcvd:
		break;
	case CS_AckRcvd:
		switch(ackNakType) {
	case Pid_Records:
		// Send first checked track header
		iter=trackList.first();
		while(iter != eol) {
			pBlock=iter;
			if(pBlock->isChecked()) break;
			iter++;
		}
		if(iter == eol) {
			// End of tracks
			sendTransferComplete(Cmnd_Transfer_Trk);
			break;
		}
		pBlock=iter;  // Don't increment
		nPacketsTrans++;
		status=pBlock->sendData(Pid_Trk_Hdr);
		if(!status) {
			lbprintf(_T("Error sending track header packet %d"),
				nPacketsTrans);
		}
#if DEBUG_UPLOAD || DEBUG_SELECT
		{
			LPWSTR unicodeString=NULL;

			ansiToUnicode(pBlock->getIdent(),&unicodeString);
			if(unicodeString) {
				lbprintf(_T("Sent track header [%d] %s"),
					Pid_Trk_Hdr,unicodeString);
				free(unicodeString);
			} else {
				lbprintf(_T("Sent track header [%d] %s"),
					Pid_Trk_Hdr,_T("<Unknown>"));
			}
		}
#endif
		progressSetProgress(_T("Tracks"),nPacketsTrans,nPackets);
		break;
	case Pid_Xfer_Cmplt:
		lbprintf(_T("  Sent %d of %d track packets"),
			nPacketsTrans,nPackets);
		progressDestroyDialog();
		commandState=CS_None;
		commandInProgress=CIP_None;
		// Restart the PVT data
		if(restartPvtData) startPVTData();
		break;
	case Pid_Trk_Hdr:
	case Pid_Trk_Data:
		while(iter != eol) {
			pBlock=iter;
			CBlockList *blockList=((CTrack *)pBlock)->getBlockList();
			// If a track header was just sent start at the
			// beginning, otherwise use the existing iter1
			if(ackNakType == Pid_Trk_Hdr) iter1=blockList->first();
			// Look for the first checked waypoint
			while(iter1 != eol) {
				pBlock1=iter1;
				if(pBlock1->isChecked()) {
					// Send the trackpoint
					nPacketsTrans++;
					status=pBlock1->sendData(Pid_Trk_Data);
					if(!status) {
						lbprintf(_T("Error sending trackpoint packet %d"),
							nPacketsTrans);
					}
#if DEBUG_UPLOAD
					{
						LPWSTR unicodeString=NULL;

						ansiToUnicode(pBlock1->getIdent(),&unicodeString);
						if(unicodeString) {
							lbprintf(_T("Sent trackpoint [%d] %s"),
								Pid_Trk_Wpt_Data,unicodeString);
							free(unicodeString);
						} else {
							lbprintf(_T("Sent trackpoint [%d] %s"),
								Pid_Trk_Wpt_Data,_T("<Unknown>"));
						}
					}
#endif
					progressSetProgress(_T("Tracks"),nPacketsTrans,nPackets);
					break;
				}
				iter1++;
			}
			// Check if we sent something, and break if so
			if(iter1 != eol) {
				iter1++;
				break;
			}
			iter++;
			// Check if we have sent all tracks
			if(iter == eol) break;
			// Send the next track header
			pBlock=iter;
			if(pBlock->isChecked()) {
				// Send the header
				nPacketsTrans++;
				status=pBlock->sendData(Pid_Trk_Hdr);
				if(!status) {
					lbprintf(_T("Error sending track header packet %d"),
						nPacketsTrans);
				}
#if DEBUG_UPLOAD
				{
					LPWSTR unicodeString=NULL;

					ansiToUnicode(pBlock->getIdent(),&unicodeString);
					if(unicodeString) {
						lbprintf(_T("Sent track header [%d] %s"),
							Pid_Trk_Hdr,unicodeString);
						free(unicodeString);
					} else {
						lbprintf(_T("Sent track header [%d] %s"),
							Pid_Trk_Hdr,_T("<Unknown>"));
					}
				}
#endif
				progressSetProgress(_T("Tracks"),
					nPacketsTrans,nPackets);
				break;
			}
		}  // End while over tracks
		// Check if we are done with the tracks
		if(iter == eol) {
			sendTransferComplete(Cmnd_Transfer_Trk);
		}
		break;
		}  // End switch on ackNakType
		break;
	case CS_NakRcvd:
		break;
	case CS_TransferComplete:
		break;
		}
		break;
	default:
		switch(commandState) {
	case CS_None:
		break;
	case CS_Started:
		break;
	case CS_RecordsRcvd:
		lbprintf(_T("  Starting %d packets"),nPackets);
		break;
	case CS_HeaderSent:
		break;
	case CS_HeaderRcvd:
		break;
	case CS_PointSent:
		break;
	case CS_PointRcvd:
		break;
	case CS_AckRcvd:
		break;
	case CS_NakRcvd:
		break;
	case CS_TransferComplete:
		lbprintf(_T("  Command complete %d"),cmdType);
		break;
		}
		break;
	}

	// If there is no command in progress, check if there is a command queued
	if(commandInProgress == CIP_None && commandQueue.count() > 0) {
		// There is a queued command.  Send it.
		CCommand *cmd=commandQueue.first();
		commandQueue.remove(*cmd);
		sendCommand(cmd->getCommand());
		switch(commandInProgress) {
	case CIP_WptUpload:
	case CIP_RteUpload:
	case CIP_TrkUpload:
		trafficController();
		}
		return;
	}
}
