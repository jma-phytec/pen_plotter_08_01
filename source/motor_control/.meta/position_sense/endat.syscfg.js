
let common = system.getScript("/common");
let hdsl_endat_pins = system.getScript("/motor_control/position_sense/hdsl_endat_pins.js");

let endat_module_name = "/motor_control/position_sense/endat";

let endat_module = {

    displayName: "EnDat Position Encoder",
    templates: {
        "/drivers/pinmux/pinmux_config.c.xdt": {
            moduleName: endat_module_name,
        },
    },
    defaultInstanceName: "CONFIG_ENDAT",
    config: [
        {
            name: "instance",
            displayName: "Instance",
            default: "ICSSG0",
            options: [
                {
                    name: "ICSSG0",
                },
                {
                    name: "ICSSG1",
                }
            ],
        },
    ],
    moduleStatic: {
        modules: function(inst) {
            return [{
                name: "system_common",
                moduleName: "/system_common",
            }]
        },
    },
    pinmuxRequirements: hdsl_endat_pins.pinmuxRequirements,
    getInterfaceName: hdsl_endat_pins.getInterfaceName,
    getPeripheralPinNames: hdsl_endat_pins.getPeripheralPinNames,
    sharedModuleInstances: sharedModuleInstances,
};

function sharedModuleInstances(instance) {
    let modInstances = new Array();

    modInstances.push({
        name: "pru",
        displayName: "PRU ICSS Configuration",
        moduleName: '/drivers/pruicss/pruicss',
        requiredArgs: {
            instance: instance.instance,
            coreClk: 200*1000000,
        },
    });

    return (modInstances);
}

exports = endat_module;
