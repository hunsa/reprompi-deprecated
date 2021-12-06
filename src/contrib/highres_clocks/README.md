# High-Resolution Clocks

Time sources for high resolution clocks

***

ReproMPI uses the Time Stamp Counter (TSC) register of the x86_64 Instruction Set Architecture (ISA) as accurate source for timing operations.

Though very accurate, unfortunately such technique is not portable, as every ISA provides a different command or register and possibly format of time values.

Following extensions are provided to use the TSC of other ISAs:

* cntpct

	Uses the Armv8 CNTPCT_EL0 (Counter-timer Physical Count) register.
	
	Before reading the time stamp, instructions in the core are serialized with the ISB command to avoid inconsistencies by out-of-order execution.
	
	Use this time source by specifying the `ENABLE_CNTPCT` flag for compilation. No frequency settings are necessary at compile time, as the Armv8 ISA supplies its own frequency information.
	
* cntvct

	Uses the Armv8 CNTVCT_EL0 (Counter-timer Virtual Count) register.
	
	This register is normally for virtual environments, but some chips (like the A64FX) provide this register as physical counter.
	
	Use this time source by specifying the `ENABLE_CNTVCT` flag for compilation.
	The same conditions as for cntpct apply.
