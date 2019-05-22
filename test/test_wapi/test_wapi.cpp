#include "pch.h"

#define CATCH_CONFIG_MAIN	// ask catch to generate a main for us
#include "..\catch2.h"

#include "tbx\wapi\AcceleratorTable.h"

using namespace tbx;
using namespace tbx::wapi;

TEST_CASE("AcceleratorTable")
{
	AcceleratorTable t;
	// TODO: we need to make this into a Windows Desktop App with resources so that we can include an accelerator table to test
}