
let common = system.getScript("/common");
let pinmux = system.getScript("/drivers/pinmux/pinmux");

function getInterfaceName(inst, peripheralName)
{
    return `PRU_${inst.instance}_${peripheralName}`;
}

function getInterfacePinList(inst, peripheralName)
{
    let interfaceName = getInterfaceName(inst, peripheralName);
    let pinList = [];

    if(peripheralName=="IEP")
    {
        if( inst.rtMode === "IRT")
        {
            pinList.push("EDC_LATCH_IN0");
            pinList.push("EDC_LATCH_IN1");
            pinList.push("EDC_SYNC_OUT0");
            pinList.push("EDC_SYNC_OUT1");
        }
    }
    else
    {
        pinList = pinmux.getInterfacePinList(interfaceName);
    }

    return pinList;
}


function getPeripheralRequirements(inst, peripheralName, name)
{
    let interfaceName = getInterfaceName(inst, peripheralName);
    let pinList = getInterfacePinList(inst, peripheralName);
    let resources = [];
    let device = common.getDeviceName();

    if(name == undefined)
    {
        name = interfaceName;
    }
    else
    {
        name = getInterfaceName(inst, name);
    }

    for(let pin of pinList)
    {
        let pinResource = pinmux.getPinRequirements(interfaceName, pin);

        /* make all pins as "rx" and then override to make "rx" as false as needed  */
        pinmux.setConfigurableDefault( pinResource, "rx", true );

        if((device === "am64x-evm") || (device === "am243x-evm"))
        {
            if((pinResource.name !== "MII0_COL") &&
               (pinResource.name !== "MII0_CRS") &&
               (pinResource.name !== "MII0_RXLINK") &&
               (pinResource.name !== "MII1_COL") &&
               (pinResource.name !== "MII1_CRS") &&
               (pinResource.name !== "MII1_RXLINK"))
            {
                resources.push( pinResource );
            }
        }
        else if(device === "am243x-lp")
        {
            if((pinResource.name !== "MII0_RXER") &&
               (pinResource.name !== "MII0_COL") &&
               (pinResource.name !== "MII0_CRS") &&
               (pinResource.name !== "MII0_RXLINK") &&
               (pinResource.name !== "MII1_RXER") &&
               (pinResource.name !== "MII1_COL") &&
               (pinResource.name !== "MII1_CRS") &&
               (pinResource.name !== "MII1_RXLINK"))
            {
                resources.push( pinResource );
            }
        }
    }

    let peripheralRequirements = {
        name: name,
        displayName: name,
        interfaceName: interfaceName,
        resources: resources,
    };

    return peripheralRequirements;
}

function pinmuxRequirements(inst) {

    let mdio = getPeripheralRequirements(inst, "MDIO");
    let iep = getPeripheralRequirements(inst, "IEP");

    /* set default values for "rx" for different pins, based on use case */
    pinmux.setPeripheralPinConfigurableDefault( mdio, "MDC", "rx", false);

    if( inst.rtMode === "IRT")
    {
        pinmux.setPeripheralPinConfigurableDefault( iep, "EDC_SYNC_OUT0", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( iep, "EDC_SYNC_OUT1", "rx", false);
    }

    if( inst.phyToMacInterfaceMode === "MII")
    {
        let mii_g_rt = getPeripheralRequirements(inst, "MII_G_RT");

        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII0_TXD0", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII0_TXD1", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII0_TXD2", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII0_TXD3", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII0_TXEN", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII1_TXD0", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII1_TXD1", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII1_TXD2", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII1_TXD3", "rx", false);
        pinmux.setPeripheralPinConfigurableDefault( mii_g_rt, "MII1_TXEN", "rx", false);

        if( inst.rtMode === "IRT")
        {
            return [mdio, iep, mii_g_rt];
        }
        else
        {
            return [mdio, mii_g_rt];
        }
    }
    else
    {
        let rgmii1 = getPeripheralRequirements(inst, "RGMII", "RGMII1");
        let rgmii2 = getPeripheralRequirements(inst, "RGMII", "RGMII2");

        if( inst.rtMode === "IRT")
        {
            return [mdio, iep, rgmii1, rgmii2];
        }
        else
        {
            return [mdio, rgmii1, rgmii2];
        }
    }
}



function getInterfaceNameList(inst) {

    if(inst.phyToMacInterfaceMode === "MII")
    {
        if( inst.rtMode === "IRT")
        {
            return [
                getInterfaceName(inst, "MDIO"),
                getInterfaceName(inst, "IEP"),
                getInterfaceName(inst, "MII_G_RT" ),
            ];
        }
        else
        {
            return [
                getInterfaceName(inst, "MDIO"),
                getInterfaceName(inst, "MII_G_RT" ),
            ];
        }
    }
    else
    {
        if( inst.rtMode === "IRT")
        {
            return [
                getInterfaceName(inst, "MDIO"),
                getInterfaceName(inst, "IEP"),
                getInterfaceName(inst, "RGMII1" ),
                getInterfaceName(inst, "RGMII2" ),
            ];
        }
        else
        {
            return [
                getInterfaceName(inst, "MDIO"),
                getInterfaceName(inst, "RGMII1" ),
                getInterfaceName(inst, "RGMII2" ),
            ];
        }
    }
}

function getPeripheralPinNames(inst)
{
    let pinList = [];

    if(inst.phyToMacInterfaceMode === "MII")
    {
        if( inst.rtMode === "IRT")
        {
            pinList = pinList.concat( getInterfacePinList(inst, "MDIO"),
                            getInterfacePinList(inst, "IEP"),
                            getInterfacePinList(inst, "MII_G_RT" )
                            );
        }
        else
        {
            pinList = pinList.concat( getInterfacePinList(inst, "MDIO"),
                            getInterfacePinList(inst, "MII_G_RT" )
                            );
        }
    }
    else
    {
        if( inst.rtMode === "IRT")
        {
            pinList = pinList.concat( getInterfacePinList(inst, "MDIO"),
                            getInterfacePinList(inst, "IEP"),
                            getInterfacePinList(inst, "RGMII" )
            );
        }
        else
        {
            pinList = pinList.concat( getInterfacePinList(inst, "MDIO"),
                            getInterfacePinList(inst, "RGMII" )
            );
        }
    }
    return pinList;
}

let profinet_module_name = "/industrial_comms/profinet/profinet";

let profinet_module = {

    displayName: "Profinet",
    templates: {
        "/drivers/pinmux/pinmux_config.c.xdt": {
            moduleName: profinet_module_name,
        },
    },
    defaultInstanceName: "CONFIG_PROFINET",
    config: [
        {
            name: "instance",
            displayName: "Instance",
            default: "ICSSG1",
            options: [
                {
                    name: "ICSSG0",
                },
                {
                    name: "ICSSG1",
                }
            ],
        },
        {
            name: "rtMode",
            displayName: "RT/IRT",
            default: "RT",
            options: [
                {
                    name: "RT",
                },
                {
                    name: "IRT",
                },
            ],
        },
        {
            name: "phyToMacInterfaceMode",
            displayName: "MII/RGMII",
            default: "MII",
            options: [
                {
                    name: "MII",
                },
                {
                    name: "RGMII",
                },
            ],
        },
        {
            name: "phyAddr0",
            description: "Phy Address of the port 0. Value MUST be between 0 .. 31",
            displayName: "Phy Address 0",
            default: 15,
        },
        {
            name: "phyAddr1",
            description: "Phy Address of Port 1. Value MUST be between 0 .. 31",
            displayName: "Phy Address 1",
            default: 3,
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
    pinmuxRequirements,
    getInterfaceNameList,
    getPeripheralPinNames,
    sharedModuleInstances: sharedModuleInstances,
    moduleInstances: moduleInstances,
};

function sharedModuleInstances(instance) {
    let modInstances = new Array();

    modInstances.push({
        name: "icss",
        displayName: "PRU Configuration",
        moduleName: '/drivers/pruicss/pruicss',
        requiredArgs: {
            instance: instance.instance,
            coreClk: 200*1000000,
        },
    });

    return (modInstances);
}

function moduleInstances(instance) {
    let modInstances = new Array();

    modInstances.push(
    {
        name: "icss_emac",
        displayName: "ICSS-EMAC Configuration (SWITCH)",
        moduleName: '/networking/icss_emac/icss_emac',
        useArray: true,
        maxInstanceCount: 1,
        minInstanceCount: 1,
        defaultInstanceCount: 1,
        requiredArgs: {
            instance: instance.instance,
            mode: "SWITCH",
            phyAddr0: instance.phyAddr0,
            phyAddr1: instance.phyAddr1,
            phyToMacInterfaceMode: instance.phyToMacInterfaceMode,
        },
    }
    );

    modInstances.push(
        {
            name: "ethphy1",
            displayName: "ETHPHY Configuration (Port 1)",
            moduleName: '/board/ethphy/ethphy',
            useArray: true,
            maxInstanceCount: 1,
            minInstanceCount: 1,
            defaultInstanceCount: 1,
            requiredArgs: {
                mdioInstance: instance.instance,
                mdioPort: instance.phyAddr0,
            },
        },
        {
            name: "ethphy2",
            displayName: "ETHPHY Configuration (Port 2)",
            moduleName: '/board/ethphy/ethphy',
            useArray: true,
            maxInstanceCount: 1,
            minInstanceCount: 1,
            defaultInstanceCount: 1,
            requiredArgs: {
                mdioInstance: instance.instance,
                mdioPort: instance.phyAddr1,
            },
        },
    );

    return (modInstances);
}

exports = profinet_module;
