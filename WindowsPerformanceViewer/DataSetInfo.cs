using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace WindowsPerformanceViewer {
    enum DataType {
        STRING,
        INT,
        DOUBLE,
        BOOLEAN,
        DATE,
    };

    /// <summary>
    /// Class to hold parameters for a data set.
    /// </summary>
    class DataSetInfo {
        private String name;
        private String units;
        private DataType type = DataType.STRING;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="name">The name.</param>
        /// <param name="units">The units.</param>
        /// <param name="type">The type.</param>
        public DataSetInfo(String name, String units, DataType type) {
            this.name = name;
            this.type = type;
            this.units = units;
        }

        // Getters and setters

        public String Name {
            get { return name; }
            set { name = value; }
        }

        public String Units {
            get { return units; }
            set { units = value; }
        }

        internal DataType Type {
            get { return type; }
            set { type = value; }
        }

    }
}
