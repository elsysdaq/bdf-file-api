#pragma once
#include <cstdint>
#include <string>

// Export/Import qualifier 
#ifdef WIN32
#ifdef FILEREADER_EXPORTS
#define BDF_API __declspec(dllexport)
#else
#define BDF_API __declspec(dllimport)
#endif
#else
#define BDF_API
#endif


#ifdef WIN32
#define FR_CC __stdcall
#else
#define FR_CC __attribute__((__stdcall__))
#endif
namespace filereader {
	/// <summary>
	///  Public Abstract BDF API Interface
	/// </summary>
	class bdfAPI {
	public:

		enum eErrorCode {
			errNoError,	/// Code for 'no error'
			errResource, /// Not enough resources available
			errArgument, /// Argument not correct
			errInvalidHandle, /// File handle is not valid
			errInternal /// Internal error
		};

		/// Operation Mode
		enum eOperationMode {
			continuous,
			singleEventRecorder,
			multiEventRecorder,
			singleEventRecorderDual,
			multiEventRecorderDual,
		};

		/// Date Structure
		struct sDateTime {
			unsigned Year;
			unsigned Month;
			unsigned Day;
			unsigned Hour;
			unsigned Minute;
			unsigned Second;
			unsigned MilliSecond;
		};

		/// Holds information about an input channel.
		struct sInputInfo {
			/// Number of bytes represanting one sample
			unsigned BytesPerSample;
			/// Mask to blind out marker bits from the binary raw data.
			unsigned AnalogMask;
			/// Mask to blind out analog bits from the binary raw data.
			unsigned MarkerMask;
			/// Number of marker bits. Markers are the rightmost bits in the sample word.
			unsigned NumberOfMarkerBits;
			/// Effective resolution of the signal in bits. 
			/** (This can be higher than the ADC resolution if averaging is switched on.)*/
			unsigned ResolutionInBits;
			/// Conversion factor binary ADC values to volt. 
			/** (volt = (binary & analogMask)*BinToVoltFactor + BinToVoltConstant.) */
			double BinToVoltFactor;
			/// Offset for conversion binary ADC values to volt. 
			double BinToVoltConstant;
			/// Conversion factor Volt to physical unit. 
			/** (physical unit = volt*VoltToPhysicalFactor + VoltToPhysicalConstant.) */
			double VoltToPhysicalFactor;
			/// Offset for conversion Volt to physical unit. 
			double VoltToPhysicalConstant;
			/// Conversion factor binary ADC values to physical unit. 
			/** (physical unit = (binary&analogMask)*BinToPhysicalFactor + BinToPhysicalConstant.) */
			double BinToPhysicalFactor;
			/// Offset for conversion binary ADC values to physical unit. 
			double BinToPhysicalConstant;
			/// Board Number
			unsigned BoardNumber;
			/// Input Number
			unsigned InputNumber;
		};

		/// Hold information about one block
		struct sBlockInfo {
			/// The number of samples witch are used to calculate one envelope min/max pair. 
			unsigned ReductionFactor;
			/// Number of envelope curves in the file. Each curves is reduced by \ref ReductionFactor from the previous curve or envelope.
			unsigned NumberOfReductions;
			/// Maximum consecutive data block size in the file. 
			unsigned PreferredTransferSize;
			/// The size of the recorded data.
			uint64_t BlockLength;
			/// Timebase from external
			bool ExternalTimebase;
			/// Sample rate in Hertz
			double SampleRateHertz;
			/// External Timebase Divisor
			unsigned TimebaseDivisor;
			/// Date and time of the start command.
			sDateTime StartTime;
			/// Trigger Time since start of recording in seconds.
			double TriggerTimeSeconds;
			/// The index of the trigger sample. 
			/** The trigger sample is the sample that is associated with time zero.*/
			uint64_t TriggerSample;
			/// The index of the stop trigger sample, for those modes that have a stop trigger.
			uint64_t StopTriggerSample;
		};



		/// <summary>
		/// Load a BDF File for Read Access Only
		/// </summary>
		/// <param name="FileName"></param>
		/// <returns>-1 on error</returns>
		virtual int loadFile(const char* FileName) = 0;

		/// <summary>
		/// Initialize a FileWriter per Group
		/// </summary>
		/// <param name="Group"></param>
		/// <param name="StartTime"></param>
		/// <param name="OperationMode"></param>
		/// <param name="SampleRate"></param>
		/// <param name="TimebaseDivisor">for Dual Mode</param>
		/// <param name="TriggerSample">Index of the Trigger Sample inside the block</param>
		/// <returns>-1 on error, GroupHandle for multi-group access</returns>
		virtual int initFileWriter(unsigned Group, sDateTime& StartTime, eOperationMode OperationMode, double SampleRate, uint32_t TimebaseDivisor, uint32_t TriggerSample) = 0;

		/// <summary>
		///  Write Input Parameter to the Input Header fields. 
		/// </summary>
		/// <param name="BoardNumber"></param>
		/// <param name="InputNumber"></param>
		/// <param name="AnalogMask">Bit Mask enabling the analog signal</param>
		/// <param name="MarkerMask">Bit Mask enabling the digital signal</param>
		/// <param name="Range"></param>
		/// <param name="Offset"></param>
		/// <param name="VoltToPhysicalFactor"></param>
		/// <param name="VoltToPhysicalConstant"></param>
		/// <param name="Handle">only used if more than 1 group present</param>
		/// <returns>eErrorCode</returns>
		virtual int writeInputHeader(uint32_t BoardNumber, uint32_t InputNumber, uint32_t AnalogMask, uint32_t MarkerMask, double Range, double Offset, double VoltToPhysicalFactor, double VoltToPhysicalConstant, int Handle = 0) = 0;

		/// <summary>
		/// Initialize a Streamer Interface per Input
		/// </summary>
		/// <param name="BoardNumber"></param>
		/// <param name="InputNumber"></param>
		/// <param name="BlockNr"></param>
		/// <param name="Handle">only used if more than 1 group present</param>
		/// <returns>StreamerHandle for write access</returns>
		virtual int initInputStreamer(uint32_t BoardNumber, uint32_t InputNumber, uint32_t BlockNr, int Handle = 0) = 0;
		
		/// <summary>
		/// Write Data to the file
		/// </summary>
		/// <param name="StreamerHandle">from the initInputStreamer function</param>
		/// <param name="Data"></param>
		/// <param name="count">in Byte</param>
		/// <param name="Handle">only used if more than 1 group present</param>
		/// <returns></returns>
		virtual int writeData(int StreamerHandle, char* Data, unsigned int count, int Handle = 0) = 0;

		/// <summary>
		/// Set strings like channel names and physical unit. These are stored as attributes key/value pairs. The following keys are used: ChName, ChPhysUnit, ChPhysUnitExt.
		/// </summary>
		/// <param name="Input">The input number (starting by 0)</param>
		/// <param name="Key">A pointer to a char array holding the key word</param>
		/// <param name="Value">A pointer to a char array witch will receivce the value</param>
		/// <param name="Size">Number of character witch will be passed to the value array</param>
		/// <param name="GroupHandle">Optional Group Handle if multiple groups are used</param>
		/// <returns></returns>
		virtual int setAttribute(unsigned Input, const std::string& Key, const std::string& Value, int GroupHandle = 0) = 0;

		/// <summary>
		/// Write the attributes which are set by the setAttribute function to the file. No more setAttribute commands are allowed after this function call. 
		/// </summary>
		/// <param name="GroupHandle">Optional Group Handle if multiple groups are used</param>
		/// <returns></returns>
		virtual int writeAttributes(int GroupHandle = 0) = 0;

		/// <summary>
		/// Write End of Record Information to the file. THis will close the corresponding block for all channels assigned to this block and switch to the next block counter
		/// </summary>
		/// <param name="BlockNr">Block Number </param>
		/// <param name="TriggerTime">Trigger time since measurement start in ps</param>
		/// <param name="DataCntr">Size in Byte of the block</param>
		/// <param name="Input"></param>
		/// <param name="Board"></param>
		/// <param name="GroupHandle"></param>
		virtual void writeEORInfo(uint32_t BlockNr, uint64_t TriggerTime, uint64_t DataCntr, uint32_t Input, uint32_t Board, int GroupHandle = 0) = 0;

		/// <summary>
		/// Close the file and rename it from *.tmp to *.bdf
		/// </summary>
		/// <param name="handle"></param>
		virtual void closeFile(int handle = -1) = 0;

		/// <summary>
		/// Initalize Reader Infrastructure, must be called after all writeInputHeader calls for all channels
		/// </summary>
		/// <returns>-1 on error</returns>
		virtual int initFileReader() = 0;

		/// <summary>
		/// Strings like channel names and physical unit are stored as attributes key/value pairs. The following keys are used: ChName, ChPhysUnit, ChPhysUnitExt.
		/// If one channel is the result of a multiplication, the value of ChPhysUnit of this channel will be xChPhysUnit* yChPhysUnitand ChPhysUnitExt is the original
		///	unit without multiplication.
		/// </summary>
		/// <param name="Group">The group address</param>
		/// <param name="Input">The input number (starting by 0)</param>
		/// <param name="Key">A pointer to a char array holding the key word</param>
		/// <param name="Value">A pointer to a char array witch will receivce the value</param>
		/// <param name="Size"> Number of character witch will be passed to the value array</param>
		/// <returns>eErrorCode</returns>
		virtual eErrorCode getAttribute(unsigned Group, unsigned Input, char* Key, char* Value, unsigned Size) = 0;

		/// <summary>
		/// All boards in one cluster are in the same group.
		/// </summary>
		/// <returns>eErrorCode</returns>
		virtual unsigned getNumberOfGroups() = 0;

		/// <summary>
		/// Get the number of inputs per group
		/// </summary>
		/// <param name="Group">number</param>
		/// <returns>eErrorCode</returns>
		virtual unsigned getNumberOfInputs(unsigned Group) = 0;

		/// <summary>
		/// Get the number of recorded blocks per input. All inputs of one group have the same number of recorded blocks, except in the ECR single channel were each input
		/// can trigger for his own.
		/// </summary>
		/// <param name="Group"></param>
		/// <param name="Input"></param>
		/// <returns>eErrorCode</returns>
		virtual unsigned getNumberOfBlocks(unsigned Group, unsigned Input) = 0;

		/// <summary>
		/// Get information about input parameters
		/// </summary>
		/// <param name="Group"></param>
		/// <param name="Input"></param>
		/// <param name="InputInfo"></param>
		/// <returns>eErrorCode</returns>
		virtual eErrorCode getInputInfo(unsigned Group, unsigned Input, sInputInfo* InputInfo) = 0;

		/// <summary>
		/// Get information about block parameters
		/// </summary>
		/// <param name="Group"></param>
		/// <param name="Input"></param>
		/// <param name="Block"></param>
		/// <param name="BlockInfo"></param>
		/// <returns>eErrorCode</returns>
		virtual eErrorCode getBlockInfo( unsigned Group, unsigned Input, unsigned Block, sBlockInfo* BlockInfo) = 0;

		/// <summary>
		/// Get Operation mode in which the file was generated
		/// </summary>
		/// <param name="Handle"></param>
		/// <param name="Group"></param>
		/// <param name="Mode"></param>
		/// <returns>eErrorCode</returns>
		virtual eErrorCode getOperationMode(unsigned Group, eOperationMode& Mode) = 0;

		/// <summary>
		/// Get the raw (binary) data in 16-bits unsigned words
		/// </summary>
		/// <param name="Group"></param>
		/// <param name="Input"></param>
		/// <param name="Block"></param>
		/// <param name="Address">The starting sample address offset inside the block</param>
		/// <param name="Data"></param>
		/// <param name="Count">in words</param>
		/// <returns>eErrorCode</returns>
		virtual eErrorCode getRawDataS(unsigned Group, unsigned Input, unsigned Block, uint64_t Address, uint16_t* Data, unsigned Count) = 0;

		/// <summary>
		/// Get the raw (binary) data in 32-bits signed words. The binary data contains also the marker bits witch must be masked out with the \ref AnalogMask. This function is slower than
		/// GetRawDataS as each sample must be convertet to a 32-bit number.
		/// </summary>
		/// <param name="Group"></param>
		/// <param name="Input"></param>
		/// <param name="Block"></param>
		/// <param name="Address">The starting sample address offset inside the block</param>
		/// <param name="Data"></param>
		/// <param name="Count"></param>
		/// <returns>eErrorCode</returns>
		virtual eErrorCode getRawDataL(unsigned Group, unsigned Input, unsigned Block, uint64_t Address, int32_t* Data, unsigned Count) = 0;

		/// <summary>
		/// Get the data scaled to voltage as floating point number
		/// </summary>
		/// <param name="Group"></param>
		/// <param name="Input"></param>
		/// <param name="Block"></param>
		/// <param name="Address">The starting sample address offset inside the block</param>
		/// <param name="Data"></param>
		/// <param name="Count"></param>
		/// <returns></returns>
		virtual eErrorCode getDataF(unsigned Group, unsigned Input, unsigned Block, uint64_t Address, float* Data, unsigned Count) = 0;

		/// <summary>
		/// Get the data scaled to voltage as double number
		/// </summary>
		/// <param name="Group"></param>
		/// <param name="Input"></param>
		/// <param name="Block"></param>
		/// <param name="Address">The starting sample address offset inside the block</param>
		/// <param name="Data"></param>
		/// <param name="Count">number of Samples to read</param>
		/// <returns>eErrorCode</returns>
		virtual eErrorCode getDataD(unsigned Group, unsigned Input, unsigned Block, uint64_t Address, double* Data, unsigned Count) = 0;

		/// <summary>
		/// Read out envelope data from a recorded block as 16bit unsigned integer. The data is scaled to volts.
		/// Set the \a Count to twice the number of min/max pairs you want to read.
		/// The data length must not be smaller than the number of min / max pairs to read.
		/// The data from \a Address to \a Address + \a BlockSize - 1 will be divided
		///	into(\a Count / 2) segments of nearly equal size and the result
		///	will contain min and max for each semgent alternately. 'Nearly equal size'
		///	means that the segments are of size(\a Count / numSegments) or
		///	(\a BlockSize / numSegments) + 1 each.
		///	
		///	Example: Read out the data from 100 to 152 and fill 5 min / max pairs.The
		///	Count parameter will be given as 5 * 2 = 10. The segments will be
		///	100..109, 110..120, 121..130, 131..141, 142..152.The array contains
		///	[min seg1, max seg1, min seg2, max seg2, ...]
		/// </summary>
		/// <param name="Group"></param>
		/// <param name="Input"></param>
		/// <param name="Block"></param>
		/// <param name="Address"></param>
		/// <param name="BlockSize"></param>
		/// <param name="Data"></param>
		/// <param name="Count"></param>
		/// <returns></returns>
		virtual eErrorCode getEnvRawDataS(unsigned Group, unsigned Input, unsigned Block, uint64_t Address, uint64_t BlockSize, uint16_t* Data, unsigned Count) = 0;

		/// <summary>
		///  Read out envelope data from a recorded block as 32-bit signed integer. The data is scaled to volts.
		/// </summary>
		/// <param name="Group"></param>
		/// <param name="Input"></param>
		/// <param name="Block"></param>
		/// <param name="Address">The starting sample address offset inside the block</param>
		/// <param name="BlockSize">The number of samples from witch the envelope is calculated</param>
		/// <param name="Data"></param>
		/// <param name="Count">The number of min/max paires divided by 2</param>
		/// <returns></returns>
		virtual eErrorCode getEnvRawDataL( unsigned Group, unsigned Input, unsigned Block, uint64_t Address, uint64_t BlockSize, int32_t* Data, unsigned Count) = 0;

		/// <summary>
		/// Read out envelope data from a recorded block as floating point numbers. The data is scaled to volts.
		/// </summary>
		/// <param name="Group"></param>
		/// <param name="Input"></param>
		/// <param name="Block"></param>
		/// <param name="Address">The starting sample address offset inside the block</param>
		/// <param name="BlockSize">The number of samples from witch the envelope is calculated</param>
		/// <param name="Data"></param>
		/// <param name="Count">The number of min/max paires divided by 2.</param>
		/// <returns>eErrorCode</returns>
		virtual eErrorCode getEnvDataF(unsigned Group, unsigned Input, unsigned Block, uint64_t Address, uint64_t BlockSize, float* Data, unsigned Count) = 0;

		/// <summary>
		/// Read out envelope data from a recorded block as double precision number. The data is scaled to volts.
		/// </summary>
		/// <param name="Group"></param>
		/// <param name="Input"></param>
		/// <param name="Block"></param>
		/// <param name="Address">The starting sample address offset inside the block</param>
		/// <param name="BlockSize">The number of samples from witch the envelope is calculated</param>
		/// <param name="Data"></param>
		/// <param name="Count">The number of min/max paires divided by 2.</param>
		/// <returns></returns>
		virtual eErrorCode getEnvDataD(unsigned Group, unsigned Input, unsigned Block, uint64_t Address, uint64_t BlockSize, double* Data, unsigned Count) = 0;

		/// <summary>
		/// Internal Release function
		/// </summary>
		virtual void Release() = 0;
	};

	/// <summary>
	/// Published API Creation Function
	/// </summary>
	extern "C" BDF_API bdfAPI * FR_CC CreateBDFAPIObj();

	/// <summary>
	/// Destroy the API Objection
	/// </summary>
	extern "C" BDF_API void FR_CC DestroyBDFAPIObj(bdfAPI * pbdfAPI);

}