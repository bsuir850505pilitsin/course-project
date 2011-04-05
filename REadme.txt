driver of the virtual hard disk drive with compression on windows 7

22.03.2011: problems with AddDevice() routine: after DriverEntry routine,instead AddDevice called DriverUnload. Without AddDevice routine in code listing, DriverUnload calls when i unload driver, but with AddDevice  driver unload immediately after DriverEntry without any error;
03.04.2011: refused from WDM and now using WDF driver model.
	Now driver successfully installing and creating virtual device.
	Pushed to repository some project files and sources.
	Began work on I/O requests.
05.04.2011:	Added I/O requests queue, read and write functions 
	(but without proccessing read-write requests, this is something like caps)
	Driver successfully installing:
DebugView: DbgPrint log
//after installing driver
00000000	0.00000000	DriverEntry routine	
00000001	0.00729227	VHDDEvtDriverDeviceAdd routine
//after uninstalling
00000002	19.19746780	VHDDEvtCleanUpCallback routine