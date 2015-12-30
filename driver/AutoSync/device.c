/*++

Copyright (c) 1990-2000  Microsoft Corporation

Module Name:

    device.c - Device handling events for example driver.

Abstract:

    This is a C version of a very simple sample driver that illustrates
    how to use the driver framework and demonstrates best practices.

--*/

#include "driver.h"




NTSTATUS
EchoDeviceCreate(
    PWDFDEVICE_INIT DeviceInit
    )
/*++

Routine Description:

    Worker routine called to create a device and its software resources.

Arguments:

    DeviceInit - Pointer to an opaque init structure. Memory for this
                    structure will be freed by the framework when the WdfDeviceCreate
                    succeeds. So don't access the structure after that point.

Return Value:

    NTSTATUS

--*/
{
    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
    PDEVICE_CONTEXT deviceContext;
    WDF_PNPPOWER_EVENT_CALLBACKS    pnpPowerCallbacks;
    WDFDEVICE device;
    NTSTATUS status;

    WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);

    //
    // Register pnp/power callbacks so that we can start and stop the timer as the device
    // gets started and stopped.
    //
    pnpPowerCallbacks.EvtDeviceSelfManagedIoInit    = EchoEvtDeviceSelfManagedIoStart;
    pnpPowerCallbacks.EvtDeviceSelfManagedIoSuspend = EchoEvtDeviceSelfManagedIoSuspend;

    #pragma prefast(suppress: 28024, "Function used for both Init and Restart Callbacks")
    pnpPowerCallbacks.EvtDeviceSelfManagedIoRestart = EchoEvtDeviceSelfManagedIoStart;

    //
    // Register the PnP and power callbacks. Power policy related callbacks will be registered
    // later in SotwareInit.
    //
    WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &pnpPowerCallbacks);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

    status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

    if (NT_SUCCESS(status)) {
        //
        // Get the device context and initialize it. WdfObjectGet_DEVICE_CONTEXT is an
        // inline function generated by WDF_DECLARE_CONTEXT_TYPE macro in the
        // device.h header file. This function will do the type checking and return
        // the device context. If you pass a wrong object  handle
        // it will return NULL and assert if run under framework verifier mode.
        //
        deviceContext = WdfObjectGet_DEVICE_CONTEXT(device);
        deviceContext->PrivateDeviceData = 0;

        //
        // Create a device interface so that application can find and talk
        // to us.
        //
        status = WdfDeviceCreateDeviceInterface(
            device,
            &GUID_DEVINTERFACE_ECHO,
            NULL // ReferenceString
            );

        if (NT_SUCCESS(status)) {
            //
            // Initialize the I/O Package and any Queues
            //
            status = EchoQueueInitialize(device);
        }
    }

    return status;
}


NTSTATUS
EchoEvtDeviceSelfManagedIoStart(
    IN  WDFDEVICE Device
    )
/*++

Routine Description:

    This event is called by the Framework when the device is started
    or restarted after a suspend operation.

    This function is not marked pageable because this function is in the
    device power up path. When a function is marked pagable and the code
    section is paged out, it will generate a page fault which could impact
    the fast resume behavior because the client driver will have to wait
    until the system drivers can service this page fault.

Arguments:

    Device - Handle to a framework device object.

Return Value:

    NTSTATUS - Failures will result in the device stack being torn down.

--*/
{
    PQUEUE_CONTEXT queueContext = QueueGetContext(WdfDeviceGetDefaultQueue(Device));
    LARGE_INTEGER DueTime;

    KdPrint(("--> EchoEvtDeviceSelfManagedIoInit\n"));

    //
    // Restart the queue and the periodic timer. We stopped them before going
    // into low power state.
    //
    WdfIoQueueStart(WdfDeviceGetDefaultQueue(Device));

    DueTime.QuadPart = WDF_REL_TIMEOUT_IN_MS(100);

    WdfTimerStart(queueContext->Timer,  DueTime.QuadPart);

    KdPrint(( "<-- EchoEvtDeviceSelfManagedIoInit\n"));

    return STATUS_SUCCESS;
}

NTSTATUS
EchoEvtDeviceSelfManagedIoSuspend(
    IN  WDFDEVICE Device
    )
/*++

Routine Description:

    This event is called by the Framework when the device is stopped
    for resource rebalance or suspended when the system is entering
    Sx state.


Arguments:

    Device - Handle to a framework device object.

Return Value:

    NTSTATUS - The driver is not allowed to fail this function.  If it does, the
    device stack will be torn down.

--*/
{
    PQUEUE_CONTEXT queueContext = QueueGetContext(WdfDeviceGetDefaultQueue(Device));

    PAGED_CODE();

    KdPrint(("--> EchoEvtDeviceSelfManagedIoSuspend\n"));

    //
    // Before we stop the timer we should make sure there are no outstanding
    // i/o. We need to do that because framework cannot suspend the device
    // if there are requests owned by the driver. There are two ways to solve
    // this issue: 1) We can wait for the outstanding I/O to be complete by the
    // periodic timer 2) Register EvtIoStop callback on the queue and acknowledge
    // the request to inform the framework that it's okay to suspend the device
    // with outstanding I/O. In this sample we will use the 1st approach
    // because it's pretty easy to do. We will restart the queue when the
    // device is restarted.
    //
    WdfIoQueueStopSynchronously(WdfDeviceGetDefaultQueue(Device));

    //
    // Stop the watchdog timer and wait for DPC to run to completion if it's already fired.
    //
    WdfTimerStop(queueContext->Timer, TRUE);

    KdPrint(( "<-- EchoEvtDeviceSelfManagedIoSuspend\n"));

    return STATUS_SUCCESS;
}



