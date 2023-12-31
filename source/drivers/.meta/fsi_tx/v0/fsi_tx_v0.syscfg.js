
let common = system.getScript("/common");
let pinmux = system.getScript("/drivers/pinmux/pinmux");
let soc = system.getScript(`/drivers/fsi_tx/soc/fsi_tx_${common.getSocName()}`);

function getStaticConfigArr() {
    return system.getScript(`/drivers/fsi_tx/soc/fsi_tx_${common.getSocName()}`).getStaticConfigArr();
}

function getInstanceConfig(moduleInstance) {
    let solution = moduleInstance[getInterfaceName(moduleInstance)].$solution;
    let staticConfigArr = getStaticConfigArr();
    let staticConfig = staticConfigArr.find( o => o.name === solution.peripheralName);

    return {
        ...staticConfig,
        ...moduleInstance
    }
};

function pinmuxRequirements(inst) {
    let interfaceName = getInterfaceName(inst);

    let resources = [];
    let pinResource = {};

    pinResource = pinmux.getPinRequirements(interfaceName, "CLK", "FSI Clock");
    pinmux.setConfigurableDefault( pinResource, "rx", false );
    resources.push( pinResource);
    pinResource = pinmux.getPinRequirements(interfaceName, "D0", "FSI TX Data 0");
    pinmux.setConfigurableDefault( pinResource, "rx", false );
    resources.push( pinResource);
    pinResource = pinmux.getPinRequirements(interfaceName, "D1", "FSI TX Data 1");
    pinmux.setConfigurableDefault( pinResource, "rx", false );
    resources.push( pinResource);

    let peripheral = {
        name          : interfaceName,
        displayName   : "FSI TX Instance",
        interfaceName : interfaceName,
        resources : resources,
    };

    return [peripheral];
}

function getInterfaceName(inst) {
    return soc.getInterfaceName(inst);
}

function getPeripheralPinNames(inst) {
    return [ "CLK", "D0", "D1" ];
}

function getClockEnableIds(inst) {
    let instConfig = getInstanceConfig(inst);
    return instConfig.clockIds;
}

function validate(inst, report) {
    /* None */
}

let fsi_tx_module_name = "/drivers/fsi_tx/fsi_tx";

let fsi_tx_module = {
    displayName: "FSI_TX",
    templates: {
        "/drivers/system/system_config.h.xdt": {
            driver_config: "/drivers/fsi_tx/templates/fsi_tx.h.xdt",
            moduleName: fsi_tx_module_name,
        },
        "/drivers/pinmux/pinmux_config.c.xdt": {
            moduleName: fsi_tx_module_name,
        },
    },
    defaultInstanceName: "CONFIG_FSI_TX",
    validate: validate,
    modules: function(inst) {
        return [{
            name: "system_common",
            moduleName: "/system_common",
        }]
    },
    pinmuxRequirements,
    getInstanceConfig,
    getInterfaceName,
    getPeripheralPinNames,
    getClockEnableIds,
};

exports = fsi_tx_module;
