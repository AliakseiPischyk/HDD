#include <iostream>
#include <iomanip>
#include <conio.h>
#include <atlstr.h>

#include "Standarts.h"

using namespace std;

const char* busType[] = { "UNKNOWN", "SCSI", "ATAPI", "ATA", "ONE_TREE_NINE_FOUR", "SSA", "FIBRE", "USB", "RAID", "ISCSI", "SAS", "SATA", "SD", "MMC" };

int main() {

	HANDLE hDevice = ::CreateFile(
		"\\\\.\\PhysicalDrive0",
		0,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	//Set the STORAGE_PROPERTY_QUERY input data structure.
	STORAGE_PROPERTY_QUERY storagePropertyQuery;
	ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	storagePropertyQuery.QueryType = PropertyStandardQuery;

	//Call DeviceIoControl once for retrieving necessary size, then allocate the output buffer.

	// Get the necessary output buffer size
	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader = { 0 };
	DWORD dwBytesReturned = 0;
	if (!::DeviceIoControl(
		hDevice,
		IOCTL_STORAGE_QUERY_PROPERTY, // dwIoControlCode. This value identifies the specific operation to be performed and the type of device on which to perform it.
		&storagePropertyQuery,
		sizeof(STORAGE_PROPERTY_QUERY),
		&storageDescriptorHeader,
		sizeof(STORAGE_DESCRIPTOR_HEADER),
		&dwBytesReturned,
		NULL))
	{
		exit(EXIT_FAILURE);
	}

	// Alloc the output buffer
	const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
	BYTE* pOutBuffer = new BYTE[dwOutBufferSize];
	ZeroMemory(pOutBuffer, dwOutBufferSize);

	//Call DeviceIoControl twice to get the storage device descriptor.
	//The output buffer points to a STORAGE_DEVICE_DESCRIPTOR structure, followed by additional info like
	//vendor ID, product ID, serial number, and so on.
	//The serial number is a null-terminated ASCII string located at SerialNumberOffset bytes counted form the beginning of the output buffer.

	 // Get the storage device descriptor
	if (!::DeviceIoControl(
		hDevice,
		IOCTL_STORAGE_QUERY_PROPERTY,
		&storagePropertyQuery,
		sizeof(STORAGE_PROPERTY_QUERY),
		pOutBuffer,
		dwOutBufferSize,
		&dwBytesReturned,
		NULL))
	{
		exit(EXIT_FAILURE);
	}

	// Now, the output buffer points to a STORAGE_DEVICE_DESCRIPTOR structure
	// followed by additional info like vendor ID, product ID, serial number, and so on.
	STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pOutBuffer;

	//Specifies the byte offset from the beginning of the structure to a null-terminated ASCII string that contains the device's vendor ID.
	//If the device has no vendor ID, this member is zero.
	const auto VendorId = pDeviceDescriptor->VendorIdOffset == 0 ? "vendor id is not specified" : CString(pOutBuffer + pDeviceDescriptor->VendorIdOffset);
	//Specifies the byte offset from the beginning of the structure to a null-terminated ASCII string that contains the device's product ID.
	//If the device has no product ID, this member is zero.
	const auto ProductId = pDeviceDescriptor->ProductIdOffset == 0 ? "product id is not specified " : CString(pOutBuffer + pDeviceDescriptor->ProductIdOffset);
	//Specifies the byte offset from the beginning of the structure to a null-terminated ASCII string that contains the device's product revision string.
	//If the device has no product revision string, this member is zero.
	const auto ProductRevision = CString(pOutBuffer + pDeviceDescriptor->ProductRevisionOffset);
	//Specifies the byte offset from the beginning of the structure to a null-terminated ASCII string that contains the device's serial number.
	//If the device has no serial number, this member is zero.
	const auto SerialNumber = CString(pOutBuffer + pDeviceDescriptor->SerialNumberOffset);
	//Specifies an enumerator value of type STORAGE_BUS_TYPE that indicates the type of bus to which the device is connected.
	//This should be used to interpret the raw device properties at the end of this structure (if any).
	const auto BusType = (char*)busType[pDeviceDescriptor->BusType];
	Standarts standarts;
	standarts.getSupportedInterfaces(hDevice);

	cout<<endl<<
		"If the device has no vendor ID, this member is zero" << endl <<
		VendorId << endl <<
		"Model"<< endl <<
		ProductId << endl <<
		"Version.If the device has no Version string, this member is zero." << endl <<
		ProductRevision << endl <<
		"If the device has no serial number, this member is zero." << endl <<
		SerialNumber << endl <<
		"Interface" << endl <<
		BusType << endl << endl;


	cout << "ATA Support:   ";
	for (int i = 8; i >= 4; i--) {
		if (standarts.ata.support[i] == 1)
			cout << "ATA" << i << " | ";
	}
	cout << endl;

	cout << "DMA Support:   ";
	for (int i = 0; i < 8; i++) {
		if (standarts.dma.support[i] == 1)
			cout << "DMA" << i << " | ";
	}
	cout << endl;

	cout << "PIO Support:   ";
	for (int i = 0; i < 2; i++) {
		if (standarts.pio.support[i] == 1)
			cout << "PIO" << i + 3 << " | ";
	}
	cout << endl;


	delete[]pOutBuffer;
	::CloseHandle(hDevice);
	system("pause");
	return EXIT_SUCCESS;
}