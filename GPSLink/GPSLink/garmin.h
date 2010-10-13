// Garmin definitions

#ifndef _INC_GARMIN_H
#define _INC_GARMIN_H

// This is the case for eTrex, ident, rte_ident, trk_ident, may not be
// true for other protocols
#define NAMELIMIT 50

typedef enum {
    Cmnd_Abort_Transfer=0, /* abort current transfer */
    Cmnd_Transfer_Alm=1, /* transfer almanac */
    Cmnd_Transfer_Posn=2, /* transfer position */
    Cmnd_Transfer_Prx=3, /* transfer proximity waypoints */
    Cmnd_Transfer_Rte=4, /* transfer routes */
    Cmnd_Transfer_Time=5, /* transfer time */
    Cmnd_Transfer_Trk=6, /* transfer track log */
    Cmnd_Transfer_Wpt=7, /* transfer waypoints */
    Cmnd_Turn_Off_Pwr=8, /* turn off power */
    Cmnd_Start_Pvt_Data=49, /* start transmitting PVT data */
    Cmnd_Stop_Pvt_Data=50, /* stop transmitting PVT data */
  // GPSLink additions
    Cmnd_Upload_Wpt=101,
    Cmnd_Upload_Rte=102,
    Cmnd_Upload_Trk=103,
    Cmnd_None=255,
} CommandType;
// NOTE: the Cmnd_Turn_Off_Pwr command may not be acknowledged by the
// GPS.

typedef enum {
    Pid_Unknown = 0,
  // Basic Packet IDs
    Pid_Ack_Byte = 6,
    Pid_Nak_Byte = 21,
    Pid_Protocol_Array = 253, // May not be implemented in all products
    Pid_Product_Rqst = 254,
    Pid_Product_Data = 255,
  //L001 Link Protocol 1
    Pid_Command_Data = 10,
    Pid_Xfer_Cmplt = 12,
    Pid_Date_Time_Data = 14,
    Pid_Position_Data = 17,
    Pid_Prx_Wpt_Data = 19,
    Pid_Records = 27,
    Pid_Rte_Hdr = 29,
    Pid_Rte_Wpt_Data = 30,
    Pid_Almanac_Data = 31,
    Pid_Trk_Data = 34,
    Pid_Wpt_Data = 35,
    Pid_Pvt_Data = 51,
    Pid_Rte_Link_Data = 98,
    Pid_Trk_Hdr = 99,
} PacketType;

typedef enum {
    Tag_Phys_Prot_Id = 'P', /* tag for Physical protocol ID */
    Tag_Link_Prot_Id = 'L', /* tag for Link protocol ID */
    Tag_Appl_Prot_Id = 'A', /* tag for Application protocol ID */
    Tag_Data_Type_Id = 'D'  /* tag for Data Type ID */
} ProtocolDataType;

typedef struct _SymbolKey {
    int index;
    char *name;
} SymbolKey;

static SymbolKey symKey[]={
    0,"White Anchor",
    1,"White Bell",
    2,"Green Diamond",
    3,"Red Diamond",
    4,"Diver Down Flag 1",
    5,"Diver Down Flag 2",
    6,"White Dollar",
    7,"White Fish",
    8,"White Fuel",
    9,"White Horn",
    10,"White House",
    11,"White Knife & Fork",
    12,"White Light",
    13,"White Mug",
    14,"White Skull & Crossbones",
    15,"Green Square",
    16,"Red Square",
    17,"White Buoy Waypoint",
    18,"Waypoint Dot",
    19,"White Wreck",
    20,"Null (transparent)",
    21,"Man Overboard",
    22,"Amber Map Buoy",
    23,"Black Map Buoy",
    24,"Blue Map Buoy",
    25,"Green Map Buoy",
    26,"Green/Red Map Buoy",
    27,"Green/White Map Buoy",
    28,"Orange Map Buoy",
    29,"Red Map Buoy",
    30,"Red/Green Map Buoy",
    31,"Red/White Map Buoy",
    32,"Violet Map Buoy",
    33,"White Map Buoy",
    34,"White/Green Map Buoy",
    35,"White/Red Map Buoy",
    36,"White Dot",
    37,"Radio Beacon",
    150,"Boat Ramp",
    151,"Campground",
    152,"Restrooms",
    153,"Shower",
    154,"Drinking Water",
    155,"Telephone",
    156,"First Aid",
    157,"Information",
    158,"Parking",
    159,"Park",
    160,"Picnic",
    161,"Scenic Area",
    162,"Skiing",
    163,"Swimming",
    164,"Dam",
    165,"Controlled Area",
    166,"Danger",
    167,"Restricted Area",
    168,"Null",
    169,"Ball",
    170,"Car",
    171,"Deer",
    172,"Shopping Cart",
    173,"Lodging",
    174,"Mine",
    175,"Trail Head",
    176,"Truck Stop",
    177,"User Exit",
    178,"Flag",
    179,"Circle with X in the Center",
    8192,"Interstate Hwy",
    8193,"US Hwy",
    8194,"State Hwy",
    8195,"Mile Marker",
    8196,"TracBack (feet)",
    8197,"Golf",
    8198,"Small City",
    8199,"Medium City",
    8200,"Large City",
    8201,"Intl Freeway Hwy",
    8202,"Intl National Hwy",
    8203,"Capitol City (Star)",
    8204,"Amusement Park",
    8205,"Bowling",
    8206,"Car Rental",
    8207,"Car Repair",
    8208,"Fast Food",
    8209,"Fitness",
    8210,"Movie",
    8211,"Museum",
    8212,"Pharmacy",
    8213,"Pizza",
    8214,"Post Office",
    8215,"RV Park",
    8216,"School",
    8217,"Stadium",
    8218,"Dept Store",
    8219,"Zoo",
    8220,"Convenience Store",
    8221,"Live Theater",
    8222,"Ramp Intersection",
    8223,"Street Intersection",
    8226,"Inspection/Weigh Station",
    8227,"Toll Booth",
    228,"Elevation Point",
    8229,"Exit without Services",
    8230,"Geographic Place Name, Man-Made",
    8231,"Geographic Place Name, Water",
    8232,"Geographic Place Name, Land",
    8233,"Bridge",
    8234,"Building",
    8235,"Cemetery",
    8236,"Church",
    8237,"Civil Location",
    8238,"Crossing",
    8239,"Historical Town",
    8240,"Levee",
    8241,"Military Location",
    8242,"Oil Field",
    8243,"Tunnel",
    8244,"Beach",
    8245,"Forest",
    8246,"Summit",
    8247,"Large Ramp Intersection",
    8248,"Large Exit without Services",
    8249,"Police/Official Badge",
    8250,"Gambling/Casino",
    8192,"Interstate Hwy",
    8193,"US Hwy",
    8194,"State Hwy",
    8195,"Mile Marker",
    8196,"TracBack (feet)",
    8197,"Golf",
    8198,"Small City",
    8199,"Medium City",
    8200,"Large City",
    8201,"Intl Freeway Hwy",
    8202,"Intl National Hwy",
    8203,"Capitol City (Star)",
    8204,"Amusement Park",
    8205,"Bowling",
    8206,"Car Rental",
    8207,"Car Repair",
    8208,"Fast Food",
    8209,"Fitness",
    8210,"Movie",
    8211,"Museum",
    8212,"Pharmacy",
    8213,"Pizza",
    8214,"Post Office",
    8215,"Rv Park",
    8216,"School",
    8217,"Stadium",
    8218,"Dept Store",
    8219,"Zoo",
    8220,"Convenience Store",
    8221,"Live Theater",
    8222,"Ramp Intersection",
    8223,"Street Intersection",
    8226,"Inspection/Weigh Station",
    8227,"Toll Booth",
    8228,"Elevation Point",
    8229,"Exit Without Services",
    8230,"Geographic Place Name, Man-Made",
    8231,"Geographic Place Name, Water",
    8232,"Geographic Place Name, Land",
    8233,"Bridge",
    8234,"Building",
    8235,"Cemetery",
    8236,"Church",
    8237,"Civil Location",
    8238,"Crossing",
    8239,"Historical Town",
    8240,"Levee",
    8241,"Military Location",
    8242,"Oil Field",
    8243,"Tunnel",
    8244,"Beach",
    8245,"Forest",
    8246,"Summit",
    8247,"Large Ramp Intersection",
    8248,"Large Exit Without Services Smbl",
    8249,"Police/Official Badge",
    8250,"Gambling/Casino",
    8251,"Snow Skiing",
    8252,"Ice Skating",
    8253,"Tow Truck (Wrecker)",
    8254,"Border Crossing (Port Of Entry)",
    16384,"Airport",
    16385,"Intersection",
    16386,"Non-Directional Beacon",
    16387,"Vhf Omni-Range",
    16388,"Heliport",
    16389,"Private Field",
    16390,"Soft Field",
    16391,"Tall Tower",
    16392,"Short Tower",
    16393,"Glider",
    16394,"Ultralight",
    16395,"Parachute",
    16396,"VOR/TACAN",
    16397,"VOR-DME",
    16398,"First Approach Fix",
    16399,"Localizer Outer Marker",
    16400,"Missed Approach Point",
    16401,"TACAN",
    16402,"Seaplane Base",
};

static SymbolKey symKeyEtrex[]={
    0,"White Anchor",
    6,"White Dollar",
    7,"White Fish",
    8,"White Fuel",
    10,"White House",
    11,"White Knife & Fork",
    14,"White Skull & Crossbones",
    18,"Waypoint Dot",
    19,"White Wreck",
    150,"Boat Ramp",
    151,"Campground",
    152,"Restrooms",
    155,"Telephone",
    156,"First Aid",
    157,"Information",
    159,"Park",
    160,"Picnic",
    161,"Scenic Area",
    162,"Skiing",
    163,"Swimming",
    170,"Car",
    171,"Deer",
    173,"Lodging",
    175,"Trail Head",
    178,"Flag",
    8197,"Golf",
    8234,"Building",
    16384,"Airport",
    16395,"Parachute",
};

#if 0
// This protocol is mainly used for marine units
typedef enum {
    Pid_Unknown = 3, // Same as ETX, guaranteed never used by Garmin
  // Basic Packet IDs
    Pid_Ack_Byte = 6,
    Pid_Nak_Byte = 21,
    Pid_Protocol_Array = 253, /* may not be implemented in all products */
    Pid_Product_Rqst = 254,
    Pid_Product_Data = 255,
  // L002 Link Protocol 2
    Pid_Almanac_Data = 4,
    Pid_Command_Data = 11,
    Pid_Xfer_Cmplt = 12,
    Pid_Date_Time_Data = 20,
    Pid_Position_Data = 24,
    Pid_Records = 35,
    Pid_Rte_Hdr = 37,
    Pid_Rte_Wpt_Data = 39,
    Pid_Wpt_Data = 43
} PacketType2;

typedef enum {
    Cmnd_Abort_Transfer = 0, /* abort current transfer */
    Cmnd_Transfer_Alm = 4, /* transfer almanac */
    Cmnd_Transfer_Rte = 8, /* transfer routes */
    Cmnd_Transfer_Time = 20, /* transfer time */
    Cmnd_Transfer_Wpt = 21, /* transfer waypoints */
    Cmnd_Turn_Off_Pwr = 26 /* turn off power */
} Protocol2Cmd;
#endif

#endif // _INC_GARMIN_H
