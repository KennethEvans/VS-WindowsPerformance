// This class is a general wrapper for MapPoint 2002.
// The class displays no error messages.  The only exceptions it catches,
// it throws again as ApplicationExceptions.
using System;
using System.Windows.Forms;

namespace MapPointUtilities
{
	/// <summary>
	/// Summary description for MapPointClient.
	/// </summary>
	public class MapPointClient : IDisposable
	{
        private MapPoint.Application app=null;
        private MapPoint.Map map=null;
        MapPoint.Location northPoleLoc=null;
        MapPoint.Location midWesternHemiLoc=null;
        private double rEarth=0;

        // Earth radius in mi Based on def. of nautical mi.
        // (Varies 3937 to 3976 mi)
        public const double REARTH=3956.0;
        // Conversion (pi/180)
        public const double DEG2RAD=0.01745329252;
        // Conversion
        public const double MI2KM=0.621371192;
        public const double MI2M=621.371192;
        
        public MapPointClient()
		{
			//
			// TODO: Add constructor logic here
			//
            Reset();
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        public void Dispose() {
            if(app != null) {
                try {
                    app.Quit();
                } catch {
                }
            }
        }

        public void Start() {
            // Open MapPoint
            try {
                app=new MapPoint.ApplicationClass();
                app.PaneState=MapPoint.GeoPaneState.geoPaneNone;
                app.Visible=true;
                app.UserControl=true;
            } catch(Exception e) {
                Reset();
                throw new ApplicationException("Could not start MapPoint",e);
            }

            // Get the map
            try {
                map=app.ActiveMap;
            } catch(Exception e) {
                Reset();
                throw new ApplicationException("Could not get MapPoint map",e);
            }

            // Add a Quit event
            try {
                ((MapPoint._ApplicationEvents_Event)app).Quit +=
                    new MapPoint._ApplicationEvents_QuitEventHandler(this.OnApplicationQuit);
            } catch(Exception e) {
                throw new ApplicationException("Could not add MapPoint Quit handler",e);
            }
        }

        public void Close() {
            if(app != null) {
                try {
                    app.Quit();
                } catch(Exception e) {
                    throw new ApplicationException("Error closing MapPoint",e);
                } finally {
                    Reset();
                }
            }
        }

        private void Reset() {
            app=null;
            map=null;
            northPoleLoc=null;
            midWesternHemiLoc=null;
            rEarth=0;
        }

        // Great circle distance in miles (Uses GPSL REARTH)        
        public static double GreatCircleDistance(double lon1, double lat1,
            double lon2, double lat2) {
            double slon,slat,a,c,d;

            // Convert to radians
            lat1*=DEG2RAD;
            lon1*=DEG2RAD;
            lat2*=DEG2RAD;
            lon2*=DEG2RAD;

            // Haversine formula
            slon=Math.Sin((lon2-lon1)/2.0);
            slat=Math.Sin((lat2-lat1)/2.0);
            a=slat*slat+Math.Cos(lat1)*Math.Cos(lat2)*slon*slon;
            c=2*Math.Atan2(Math.Sqrt(a),Math.Sqrt(1-a));
            d=REARTH*c;

            return(d);
        }

        // Great circle distance in miles (Uses MapPoint rEarth=3963.229899 mi)
        public double MapPointGreatCircleDistance(double lon1, double lat1,
            double lon2, double lat2) {
            double slon,slat,a,c,d;

            // Calculate rEarth if not done yet
            if(rEarth == 0.0) CalculateMapPointREarth();

            // Convert to radians
            lat1*=DEG2RAD;
            lon1*=DEG2RAD;
            lat2*=DEG2RAD;
            lon2*=DEG2RAD;

            // Haversine formula
            slon=Math.Sin((lon2-lon1)/2.0);
            slat=Math.Sin((lat2-lat1)/2.0);
            a=slat*slat+Math.Cos(lat1)*Math.Cos(lat2)*slon*slon;
            c=2*Math.Atan2(Math.Sqrt(a),Math.Sqrt(1-a));
            d=rEarth*c;

            return(d);
        }

        public struct LonLatPtD {
            public double Lat;
            public double Lon;
        }

        // A workaround for MapPoint's not supplying a function to get the
        // Lat/Lon for a Location
        public LonLatPtD GetLatLonFromLocation(MapPoint.Location loc) {
            // Calculate rEarth if not done yet
            if(rEarth == 0.0) CalculateMapPointREarth();
            LonLatPtD point=new LonLatPtD();
            point.Lat=90.0-180.0/(Math.PI*rEarth)*map.Distance(northPoleLoc,loc);
            MapPoint.Location greenLoc=map.GetLocation(point.Lat,0,100);
            double dist=map.Distance(loc,greenLoc);
            double theta=point.Lat*DEG2RAD;
#if false
            double phi=(Math.Cos(2.0*Math.PI*dist/CEarth)-
                Math.Sin(theta)*Math.Sin(theta))/(Math.Cos(theta)*Math.Cos(theta));
            // Use - for North America.  Fix it up to use in Eastern hemisphere
            point.Lon=-Math.Acos(phi)/DEG2RAD;
#else
            double phi=Math.Sin(.5*dist/rEarth)/Math.Cos(theta);
            point.Lon=2*Math.Asin(phi)/DEG2RAD;
            // If the distance from the point at (0,-90) is less than
            // .25 circumfrence, then it is in the Western hemisphere
            if(map.Distance(midWesternHemiLoc,loc) < .5*Math.PI*rEarth) {
                point.Lon=-point.Lon;
            }
#endif

            return point;
        }

        // Calculate rEarth and northPoleLoc once
        private void CalculateMapPointREarth() {
            // Throw an exception if MapPoint is not started
            if(!Started) {
                throw new ApplicationException("MapPoint REarth cannot" +
                    " be determined until MapPoint is started");
            }

            // Only calculate it if it hasn't been
            if(rEarth == 0) {
                northPoleLoc=map.GetLocation(90,0,100);
                midWesternHemiLoc=map.GetLocation(0,-90,100);
                MapPoint.Location southPoleLoc=map.GetLocation(-90,0,100);
                rEarth=map.Distance(northPoleLoc,southPoleLoc)/Math.PI;
            }
        }

        public MapPoint.Application App {
            get { return app; }
        }

        public MapPoint.Map Map {
            get { return map; }
        }

        public double MapPointREarth {
            get { if(REarth == 0) CalculateMapPointREarth(); return REarth; }
        }

        public double REarth {
            get { return REARTH; }
        }

        public bool Started {
            get { return (app == null)?false:true; }
        }

        public double Altitude {
            get { return map.Altitude; }
            set { map.Altitude=value; }
        }

        public MapPoint.Location Location {
            get { return map.Location; }
            set { map.Location=value; }
        }

        public int Width {
            get { return map != null?map.Width:-1; }
            set {
                if(map == null) return;
                int appWidth=app.Width;
                int mapWidth=map.Width;
                app.Width=appWidth-mapWidth+value;
            }
        }

        public int Height {
            get { return map != null?map.Height:-1; }
            set {
                if(map == null) return;
                int appHeight=app.Height;
                int mapHeight=map.Height;
                app.Height=appHeight-mapHeight+value;
            }
        }

        private void OnApplicationQuit() {
            Reset();
        }
    }
}
