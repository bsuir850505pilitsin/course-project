driver of the virtual hard drive with compression on windows

22.03.2011: problems with AddDevice() routine: after DriverEntry routine,instead AddDevice called DriverUnload. Without AddDevice routine in code listing, DriverUnload calls when i unload driver, but with AddDevice  driver unload immediately after DriverEntry without any error;