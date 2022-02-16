const {
    fromZigbeeConverters,
    toZigbeeConverters,
    exposes
} = require('zigbee-herdsman-converters');

const ep = exposes.presets;
const ea = exposes.access;

const bind = async (endpoint, target, clusters) => {
    for (const cluster of clusters) {
        await endpoint.bind(cluster, target);
    }
};

const configureReporting = {
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
              result.mode = ['Never', 'Once', 'Always', 'Drop'][msg.data[0x0051]];
              result.once = 'OFF';
              result.always = 'OFF';
              result.drop = 'OFF';
              if (msg.data[0x0051] == 1) {
                result.once = 'ON';
              }
              if (msg.data[0x0051] == 2) {
                result.always = 'ON';
              }
              if (msg.data[0x0051] == 3) {
                result.drop = 'ON';
              }
          }
          if (msg.data.hasOwnProperty(0x0052)) {
              result.sound = ['OFF', 'ON'][msg.data[0x0052]];
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
          if (msg.data.hasOwnProperty(0x0057)) {
              result.time_bell = msg.data[0x0057];
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
      key: ['state', 'mode', 'sound', 'once', 'always', 'drop', 'time_ring', 'time_talk', 'time_open', 'time_bell', 'time_report'],
      convertSet: async (entity, key, rawValue, meta) => {
          const lookup = {
              'OFF': '0',
              'ON': '1',
          };
          const modeOpenLookup = {
              'Never': '0',
              'Once': '1',
              'Always': '2',
              'Drop': '3',
          };


          let value = lookup.hasOwnProperty(rawValue) ? lookup[rawValue] : parseInt(rawValue, 10);

          if (key == 'mode') {
              value = modeOpenLookup.hasOwnProperty(rawValue) ? modeOpenLookup[rawValue] : parseInt(rawValue, 10);
          }

          if (key == 'once') {
              value = (rawValue == 'ON') ? 1 : 0;
          }

          if (key == 'always') {
              value = (rawValue == 'ON') ? 2 : 0;
          }

          if (key == 'drop') {
              value = (rawValue == 'ON') ? 3 : 0;
          }

          const payloads = {
              mode: {0x0051: {value, type: 0x30}},
              sound: {0x0052: {value, type: 0x10}},
              once: {0x0051: {value, type: 0x30}},
              always: {0x0051: {value, type: 0x30}},
              drop: {0x0051: {value, type: 0x30}},
              time_ring: {0x0053: {value, type: 0x20}},
              time_talk: {0x0054: {value, type: 0x20}},
              time_open: {0x0055: {value, type: 0x20}},
              time_bell: {0x0057: {value, type: 0x20}},
              time_report: {0x0056: {value, type: 0x20}},
          };

          await entity.write('closuresDoorLock', payloads[key]);

          if (key == 'once' || key == 'always' || key == 'drop' || key == 'mode') {
              const payloads = {
                  mode: ['closuresDoorLock', 0x0051],
                  once: ['closuresDoorLock', 0x0051],
                  always: ['closuresDoorLock', 0x0051],
                  drop: ['closuresDoorLock', 0x0051],
              };
              await entity.read(payloads[key][0], [payloads[key][1]]);
          }

          return {
              state: {[key]: rawValue},
          };
      },
      convertGet: async (entity, key, meta) => {
          const payloads = {
              state: ['closuresDoorLock', 0x0050],
              mode: ['closuresDoorLock', 0x0051],
              sound: ['closuresDoorLock', 0x0052],
              once: ['closuresDoorLock', 0x0051],
              always: ['closuresDoorLock', 0x0051],
              drop: ['closuresDoorLock', 0x0051],
              time_ring: ['closuresDoorLock', 0x0053],
              time_talk: ['closuresDoorLock', 0x0054],
              time_open: ['closuresDoorLock', 0x0055],
              time_bell: ['closuresDoorLock', 0x0057],
              time_report: ['closuresDoorLock', 0x0056],
          };
          await entity.read(payloads[key][0], [payloads[key][1]]);
      },
  },
}

const device = {
    zigbeeModel: ['DIY_Zintercom'],
    model: 'DIYRuZ_Zintercom',
    vendor: 'DIYRuZ',
    description: '[Matrix intercom auto opener](https://diyruz.github.io/posts/zintercom/)',
    icon: 'https://raw.githubusercontent.com/diyruz/Zintercom/master/images/z2m.png',
    fromZigbee: [
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

        await bind(firstEndpoint, coordinatorEndpoint, ['closuresDoorLock', 'genPowerCfg']);

        const overides = {min: 0, max: 3600, change: 0};
        await configureReporting.batteryVoltage(firstEndpoint, overides);
        await configureReporting.batteryPercentageRemaining(firstEndpoint, overides);

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

    },
    exposes: [
        exposes.enum('state', ea.STATE_GET, ['Idle', 'Ring', 'Talk', 'Open', 'Drop'])
            .withDescription('Current state'),
        exposes.enum('mode', ea.ALL, ['Never', 'Once', 'Always', 'Drop'])
            .withDescription('Select open mode'),
        exposes.binary('sound', ea.ALL, 'ON', 'OFF').withProperty('sound')
            .withDescription('Enable or disable sound'),
        exposes.binary('once', ea.ALL, 'ON', 'OFF').withProperty('once')
            .withDescription('Enable or disable once mode'),
        exposes.binary('always', ea.ALL, 'ON', 'OFF').withProperty('always')
            .withDescription('Enable or disable always mode'),
        exposes.binary('drop', ea.ALL, 'ON', 'OFF').withProperty('drop')
            .withDescription('Enable or disable drop mode'),
        exposes.numeric('time_ring', ea.ALL).withUnit('sec')
            .withDescription('Time to ring before answer'),
        exposes.numeric('time_talk', ea.ALL).withUnit('sec')
            .withDescription('Time to hold before open'),
        exposes.numeric('time_open', ea.ALL).withUnit('sec')
            .withDescription('Time to open before end'),
        exposes.numeric('time_bell', ea.ALL).withUnit('sec')
            .withDescription('Time after last bell to finish ring'),
        exposes.numeric('time_report', ea.ALL).withUnit('min')
            .withDescription('Reporting interval'),
        ep.battery(),
    ],
};

module.exports = device;
