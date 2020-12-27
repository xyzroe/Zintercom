const {
    fromZigbeeConverters,
    toZigbeeConverters,
    exposes
} = require('zigbee-herdsman-converters');

const e = exposes.presets;

const ZCL_DATATYPE_INT16 = 0x29;
const ZCL_DATATYPE_UINT8 = 0x20;
const ZCL_DATATYPE_UINT16 = 0x21;
const ZCL_DATATYPE_BOOLEAN = 0x10;
const ZCL_DATATYPE_INT32 = 0x2b;
const bind = async (endpoint, target, clusters) => {
    for (const cluster of clusters) {
        await endpoint.bind(cluster, target);
    }
};

const configureReporting = {
    currentPositionLiftPercentage: async (endpoint, overrides) => {
        const payload = configureReportingPayload('currentPositionLiftPercentage', 1, repInterval.MAX, 1, overrides);
        await endpoint.configureReporting('closuresWindowCovering', payload);
    },
    batteryPercentageRemaining: async (endpoint, overrides) => {
        const payload = configureReportingPayload(
            'batteryPercentageRemaining', repInterval.HOUR, repInterval.MAX, 0, overrides,
        );
        await endpoint.configureReporting('genPowerCfg', payload);
    },
    batteryVoltage: async (endpoint, overrides) => {
        const payload = configureReportingPayload('batteryVoltage', repInterval.HOUR, repInterval.MAX, 0, overrides);
        await endpoint.configureReporting('genPowerCfg', payload);
    },
}

const configureReportingPayload = (attribute, min, max, change, overrides) => {
    const payload = {
        attribute: attribute,
        minimumReportInterval: min,
        maximumReportInterval: max,
        reportableChange: change,
    };


    if (overrides) {
        if (overrides.hasOwnProperty('min')) payload.minimumReportInterval = overrides.min;
        if (overrides.hasOwnProperty('max')) payload.maximumReportInterval = overrides.max;
        if (overrides.hasOwnProperty('change')) payload.reportableChange = overrides.change;
    }

    return [payload];
};

const repInterval = {
    MAX: 62000,
    HOUR: 3600,
    MINUTES_30: 1800,
    MINUTES_15: 900,
    MINUTES_10: 600,
    MINUTES_5: 300,
    MINUTE: 60,
};
//const ACCESS_STATE = 0b001, ACCESS_WRITE = 0b010, ACCESS_READ = 0b100;

const hass = {
    co2: {
        type: 'sensor',
        object_id: 'co2',
        discovery_payload: {
            unit_of_measurement: 'ppm',
            icon: 'mdi:molecule-co2',
            value_template: '{{ value_json.co2 }}',
        },
    },
    temperature: {
        type: 'sensor',
        object_id: 'temperature',
        discovery_payload: {
            unit_of_measurement: '°C',
            device_class: 'temperature',
            value_template: '{{ value_json.temperature }}',
        },
    },
    humidity: {
        type: 'sensor',
        object_id: 'humidity',
        discovery_payload: {
            unit_of_measurement: '%',
            device_class: 'humidity',
            value_template: '{{ value_json.humidity }}',
        },
    },
    presure: {
        type: 'sensor',
        object_id: 'pressure',
        discovery_payload: {
            unit_of_measurement: 'hPa',
            device_class: 'pressure',
            value_template: '{{ value_json.pressure }}',
        },
    }
};



const fz = {
  diy_zintercom_config: {
      cluster: 'closuresDoorLock',
      type: ['attributeReport', 'readResponse'],
      convert: (model, msg, publish, options, meta) => {
          const result = {};
          if (msg.data.hasOwnProperty(0x0050)) {
              result.state = ['Idle', 'Ring', 'Talk', 'Open', 'Drop'][msg.data[0x0050]];
          }
          if (msg.data.hasOwnProperty(0x0051)) {
              result.mode_open = ['Never', 'Once', 'Always', 'Drop'][msg.data[0x0051]];
          }
          if (msg.data.hasOwnProperty(0x0052)) {
              result.mode_sound = ['OFF', 'ON'][msg.data[0x0052]];
          }
          if (msg.data.hasOwnProperty(0x0053)) {
              result.time_ring = msg.data[0x0053];
          }
          if (msg.data.hasOwnProperty(0x0054)) {
              result.time_talk = msg.data[0x0054];
          }
          if (msg.data.hasOwnProperty(0x0055)) {
              result.time_open = msg.data[0x0055];
          }
          if (msg.data.hasOwnProperty(0x0056)) {
              result.time_report = msg.data[0x0056];
          }
          return result;
      },
  },
}

const tz = {
  diy_zintercom_config: {
      key: ['state', 'mode_open', 'mode_sound', 'time_ring', 'time_talk', 'time_open', 'time_report'],
      convertSet: async (entity, key, rawValue, meta) => {
          const lookup = {
              'OFF': 0x00,
              'ON': 0x01,
          };
          const modeOpenLookup = {
              'Never': '0',
              'Once': '1',
              'Always': '2',
              'Drop': '3',
          };


          let value = lookup.hasOwnProperty(rawValue) ? lookup[rawValue] : parseInt(rawValue, 10);

          if (key == 'mode_open') {
              value = modeOpenLookup.hasOwnProperty(rawValue) ? modeOpenLookup[rawValue] : parseInt(rawValue, 10);
          }

          const payloads = {
              mode_open: {0x0051: {value, type: 0x30}},
              mode_sound: {0x0052: {value, type: 0x10}},
              time_ring: {0x0053: {value, type: 0x20}},
              time_talk: {0x0054: {value, type: 0x20}},
              time_open: {0x0055: {value, type: 0x20}},
              time_report: {0x0056: {value, type: 0x20}},
          };

          await entity.write('closuresDoorLock', payloads[key]);
          return {
              state: {[key]: rawValue},
          };
      },
      convertGet: async (entity, key, meta) => {
          const payloads = {
              state: ['closuresDoorLock', 0x0050],
              mode_open: ['closuresDoorLock', 0x0051],
              mode_sound: ['closuresDoorLock', 0x0052],
              time_ring: ['closuresDoorLock', 0x0053],
              time_talk: ['closuresDoorLock', 0x0054],
              time_open: ['closuresDoorLock', 0x0055],
              time_report: ['closuresDoorLock', 0x0056],
          };
          await entity.read(payloads[key][0], [payloads[key][1]]);
      },
  },
}

const device = {
    zigbeeModel: ['DIY_Zintercom'],
    model: 'DIY_Zintercom',
    vendor: 'xyzroe',
    description: '[Intercom Auto Opener]',
    supports: '',
    //homeassistant: [hass.temperature, hass.presure, hass.humidity, hass.co2],
    fromZigbee: [
      /*
        fromZigbeeConverters.temperature,
        fromZigbeeConverters.humidity,
        fromZigbeeConverters.co2,
        fromZigbeeConverters.pressure,
        */
        fromZigbeeConverters.battery,
        fz.diy_zintercom_config,
    ],
    toZigbee: [
        toZigbeeConverters.factory_reset,
        tz.diy_zintercom_config,
    ],
    meta: {
        configureKey: 1,
    },
    configure: async (device, coordinatorEndpoint) => {
        const firstEndpoint = device.getEndpoint(1);

        //await bind(firstEndpoint, coordinatorEndpoint, ['msCO2', 'closuresDoorLock', 'genOnOff']);

        await bind(firstEndpoint, coordinatorEndpoint, ['closuresDoorLock', 'genPowerCfg']);
        const overides = {min: 0, max: 3600, change: 0};
        await configureReporting.batteryVoltage(firstEndpoint, overides);
        await configureReporting.batteryPercentageRemaining(firstEndpoint, overides);
/*
        if (device.applicationVersion < 3) { // Legacy PM2 firmwares
            const payload = [{
                attribute: 'batteryPercentageRemaining',
                minimumReportInterval: 0,
                maximumReportInterval: 3600,
                reportableChange: 0,
            }, {
                attribute: 'batteryVoltage',
                minimumReportInterval: 0,
                maximumReportInterval: 3600,
                reportableChange: 0,
            }];
            await firstEndpoint.configureReporting('genPowerCfg', payload);
        }
        */
/*
        const msBindPayload = [{
            attribute: 'measuredValue',
            minimumReportInterval: 0,
            maximumReportInterval: 3600,
            reportableChange: 0,
        }];
        await firstEndpoint.configureReporting('msCO2', msBindPayload);
*/

        const payload = [{
            attribute: {
                ID: 0x0050,
                type: 0x30,
            },
            minimumReportInterval: 0,
            maximumReportInterval: 3600,
            reportableChange: 0,
        },
      ];
        await firstEndpoint.configureReporting('closuresDoorLock', payload);
        /**/
        /*
        await firstEndpoint.configureReporting('msTemperatureMeasurement', msBindPayload);
        await firstEndpoint.configureReporting('msRelativeHumidity', msBindPayload);

        const pressureBindPayload = [{
            attribute: 'scaledValue',
            minimumReportInterval: 0,
            maximumReportInterval: 3600,
            reportableChange: 0,
        }];
        await firstEndpoint.configureReporting('msPressureMeasurement', pressureBindPayload);
        */


    },
    exposes: [
        //exposes.numeric('co2', ACCESS_STATE).withUnit('ppm'),
        e.battery(),
        exposes.enum('state', exposes.access.STATE_GET, ['Idle', 'Ring', 'Talk', 'Open', 'Drop'])
            .withDescription('Current state'),
        exposes.enum('mode_open', exposes.access.ALL, ['Never', 'Once', 'Always', 'Drop'])
            .withDescription('Auto open mode'),
        exposes.binary('mode_sound', exposes.access.ALL, 'ON', 'OFF')
            .withDescription('Sound mode'),
        exposes.numeric('time_ring', exposes.access.ALL).withUnit('sec')
            .withDescription('Time to ring'),
        exposes.numeric('time_talk', exposes.access.ALL).withUnit('sec')
            .withDescription('Time to "speak" before open'),
        exposes.numeric('time_open', exposes.access.ALL).withUnit('sec')
            .withDescription('Time to "hold open button"'),
        exposes.numeric('time_report', exposes.access.ALL).withUnit('min')
            .withDescription('Reporting interval'),
    ],
};

module.exports = device;
