# Project 3 - OpenStreetMap and CSV Bus System

Student: Shantanu (921200468)

## Overview

Implementation of CCSVBusSystem, which parses delimiter-separated value files containing bus stop and route data into an in-memory representation conforming to the CBusSystem interface. Also includes COpenStreetMap (implemented by partner), which parses OpenStreetMap XML data into a CStreetMap structure. Both classes use the pimpl pattern and rely on CDSVReader and CXMLReader from Project 2.

## Build and Run

```
make        # compiles and runs all tests
make clean  # removes all build artifacts
```

## Code References

* [C++ Reference Documentation](http://www.cplusplus.com/reference/) - reference for C++ built-in functions and classes
* [OpenStreetMap XML Format](https://wiki.openstreetmap.org/wiki/OSM_XML) - OSM XML structure referenced in project spec
* [RFC 4180 - CSV Format](https://tools.ietf.org/html/rfc4180) - standard conventions for CSV field structure
* ECS34 guides provided on Canvas

## Generative AI Use

- AI-assisted tab completion (IDE autocomplete) - explicitly permitted per course policy
- Consulted generative AI assistant for implementation guidance and debugging

---

### 1. Makefile Structure for Multiple Test Executables

**Prompt:** "How do I extend my existing Makefile to support new test executables for CCSVBusSystem and COpenStreetMap, where each needs different object file dependencies?"

**Original AI Response:**
"Add a new test executable target for each class following the same pattern as your existing targets. For CCSVBusSystem, link together the CSVBusSystem object, DSVReader object, StringDataSource, StringDataSink, and the test object file. For COpenStreetMap, link XMLReader, OpenStreetMap, StringDataSource, StringDataSink, and the test object, and add `-lexpat` since XMLReader depends on it. Add both new executables to your `test` target dependencies and add their run commands. The pattern rules for obj/%.o and testobj/%.o already handle compilation automatically, so you only need the new link targets."

**Changes Made:** Added `testbin/testcsvbs` and `testbin/testosm` as new link targets with their correct object dependencies. Added `-lexpat` only to the OpenStreetMap target since only XMLReader depends on it. Added both to the `test` target dependency list and run commands.

**Rationale:** AI confirmed the existing pattern rules cover new source files automatically; I only needed the link targets and verified the correct object file dependencies by tracing the include chain.

---

### 2. Conditional Homebrew Paths for macOS and CSIF

**Prompt:** "I need the Makefile to detect whether it's running on my Mac with Homebrew or on the CSIF Linux server where libraries are in standard system paths."

**Original AI Response:**
"Use GNU Make's wildcard function inside an ifneq conditional. Check for a header file that only exists when Homebrew is installed, like `/opt/homebrew/include/gtest/gtest.h`. If the file exists, set EXTRA_INC and EXTRA_LIB to the Homebrew paths. If it doesn't, leave those variables empty. Since CSIF has gtest and expat installed at standard system locations, an empty EXTRA_INC and EXTRA_LIB means the compiler uses its default search paths, which is exactly what you want."

**Changes Made:** Copied the conditional Homebrew detection block from Project 2's Makefile and kept it as-is since the same libraries are needed for this project.

**Rationale:** The approach already worked in Project 2 with the same libraries; reusing it without changes was the right call since the dependency set is identical.

---

### 3. Pimpl Pattern for CCSVBusSystem

**Prompt:** "The header file uses a private SImplementation struct with a unique_ptr. How do I set this up correctly in the .cpp file?"

**Original AI Response:**
"Define the SImplementation struct fully inside the .cpp file, not the header. This is the whole point of pimpl — the header only declares that SImplementation exists, while the .cpp defines its contents. In the .cpp, write `struct CCSVBusSystem::SImplementation { ... }` with all your data members inside. In the constructor, initialize DImplementation with `std::make_unique<SImplementation>()`. You'll need to access all implementation state through `DImplementation->` in every method. The destructor can be defaulted with `= default` in the .cpp since unique_ptr handles cleanup automatically."

**Changes Made:** Defined the full SImplementation struct at the top of CSVBusSystem.cpp with vectors and maps as members. Initialized it in the constructor with make_unique and used DImplementation-> to access all state in every method. Added `= default` destructor in the .cpp.

**Rationale:** AI clarified that the struct definition belongs in the .cpp, not the header; keeping it there ensures the header stays stable and the implementation details stay hidden.

---

### 4. Inner Structs Implementing Virtual Interfaces

**Prompt:** "CBusSystem::SStop and SRoute are abstract structs with pure virtual methods. How do I define concrete implementations of them inside SImplementation?"

**Original AI Response:**
"Define concrete structs inside SImplementation that inherit from the abstract base structs. For example, `struct SStop : public CBusSystem::SStop { ... }`. Add data members for the actual values (like DID and DNodeID) and implement each pure virtual method to return the appropriate member. Since SStop is polymorphic, instances need to be heap-allocated. Store them as `shared_ptr<SStop>` in your vector — note this is a shared_ptr to the concrete type, which is compatible with returning `shared_ptr<CBusSystem::SStop>` from the public methods since the concrete type inherits from the base."

**Changes Made:** Defined SStop and SRoute as nested structs inside SImplementation, each inheriting from the corresponding CBusSystem base struct. Stored them in vectors of shared_ptr to the concrete type and returned them directly from StopByIndex etc., relying on implicit pointer conversion from derived to base.

**Rationale:** AI explained the inheritance and pointer compatibility; nesting the concrete structs inside SImplementation keeps them fully private to the implementation file.

---

### 5. Reading CSV Header Rows

**Prompt:** "The stops.csv and routes.csv files have a header row. How do I skip it?"

**Original AI Response:**
"Call ReadRow once before your main parsing loop to consume the header. Don't use the result — just discard it. Then your while loop reads actual data rows. This is simpler than checking field names inside the loop. Just make sure to check End() before your loop starts in case the file is completely empty after the header."

**Changes Made:** Added a single `stopsrc->ReadRow(Row)` call before each while loop to consume the header. The while loop then only processes data rows.

**Rationale:** AI confirmed the discard approach; it's simpler than adding a row counter or checking field names inside the loop.

---

### 6. Using unordered_map for ID Lookups

**Prompt:** "What's the best data structure for looking up a bus stop by its integer ID in O(1) time?"

**Original AI Response:**
"Use std::unordered_map with the stop ID as the key and the vector index as the value. When you insert a stop into your vector, also insert its ID mapped to its index into the unordered_map. Then StopByID becomes a map lookup followed by a vector index, both O(1). unordered_map uses hashing so average lookup is O(1), compared to O(log n) for std::map. Since TStopID is uint64_t, it has a built-in hash specialization, so you don't need a custom hasher."

**Changes Made:** Added `std::unordered_map<TStopID, std::size_t> DStopByID` and `std::unordered_map<std::string, std::size_t> DRouteByName` to SImplementation. Populated both during construction alongside their respective vectors.

**Rationale:** AI confirmed uint64_t has a default hash; using the vector index as the map value avoids storing duplicate data and keeps the two structures in sync.

---

### 7. Parsing Integer IDs from CSV Strings

**Prompt:** "The stop IDs in the CSV are strings like \"22043\". How do I convert them to uint64_t?"

**Original AI Response:**
"Use `std::stoull` which converts a string to unsigned long long, which is 64-bit on all modern platforms. It's the unsigned equivalent of stoll. For very large node IDs like OpenStreetMap uses, stoull handles values up to 2^64-1 which covers the full OSM ID range. Just make sure to include `<string>` since stoull lives in that header."

**Changes Made:** Used `std::stoull(Row[0])` and `std::stoull(Row[1])` to parse stop IDs and node IDs from the CSV row strings.

**Rationale:** AI pointed out stoull handles the full uint64_t range; stoul would be unsafe for OSM node IDs which can be large enough to overflow 32 bits.

---

### 8. Preserving Route Stop Insertion Order

**Prompt:** "A route like \"A\" appears across many rows in routes.csv, each with a different stop_id. How do I group them correctly while preserving order?"

**Original AI Response:**
"Check whether the route name already exists in your name-to-index map. If it does, retrieve the existing route object using the stored index and append the stop ID to its stop list. If it doesn't, create a new route object with the first stop, add it to the vector, and insert the name and new index into the map. This gives you O(1) lookup per row and preserves the exact insertion order since you're appending rather than sorting."

**Changes Made:** Used `DRouteByName.find(RouteName)` to check for an existing route on each row. If found, appended to the existing route's DStopIDs vector. If not found, created a new SRoute, added it to DRoutes, and inserted the mapping.

**Rationale:** AI confirmed the append approach preserves order naturally; no sorting step is needed since routes.csv lists stops in sequence.

---

### 9. GetStopID Out-of-Bounds Handling

**Prompt:** "What should GetStopID return if the index is out of range?"

**Original AI Response:**
"Return the sentinel value for an invalid stop ID. In CBusSystem, this is defined as `InvalidStopID`, which is `std::numeric_limits<TStopID>::max()`. This is the standard pattern for the interface — callers check whether the returned ID equals InvalidStopID to detect an invalid result. Just add a bounds check before accessing the vector."

**Changes Made:** Added `if(index < DStopIDs.size())` before the vector access in GetStopID. The else branch returns `CBusSystem::InvalidStopID`.

**Rationale:** AI identified the correct sentinel value from the interface; bounds-checking before vector access prevents undefined behavior on out-of-range indices.

---

### 10. Testing with In-Memory CSV Strings

**Prompt:** "How do I write unit tests for CSVBusSystem without relying on external files, so I can test exact behavior with controlled input?"

**Original AI Response:**
"Construct your test CSV data as a string literal directly in the test. Then wrap it in a CStringDataSource and pass it to CDSVReader, just like you would with a file-based source. This lets you define exactly what the input is, test edge cases like empty files, single-stop routes, and specific stop orderings, and verify results without any file I/O dependency. Since CStringDataSource accepts any std::string, you can inline the test data right in the EXPECT calls if needed."

**Changes Made:** Wrote all tests using string literals for StopsCSV and RoutesCSV. These cover exact counts, stop lookup by ID, route stop ordering, out-of-bounds GetStopID, and empty input.

**Rationale:** AI confirmed that CStringDataSource works identically to a file-backed source; in-memory tests run faster, make expected values obvious, and don't depend on file paths being correct at test time.

---

### 11. Test Helper Function to Reduce Repetition

**Prompt:** "Every test in my CSVBusSystem test file constructs the system the same way with two CDSVReader arguments. Is there a cleaner way to handle this?"

**Original AI Response:**
"Extract the construction into a static helper function at the top of the test file. It takes the two CSV strings and returns a CCSVBusSystem by value. Since CCSVBusSystem uses a unique_ptr internally, make sure it's movable or construct it in place. This keeps each test focused on what it's actually testing rather than repeating boilerplate setup."

**Changes Made:** Added a `MakeSystem` static helper function at the top of CSVBusSystemTest.cpp that takes two strings and returns a CCSVBusSystem. Each test calls it in one line instead of repeating the full construction.

**Rationale:** AI suggested the helper pattern; it significantly reduces repeated code and makes each test easier to read.

---

### 12. std::numeric_limits Instead of CBusSystem::InvalidStopID in Tests

**Prompt:** "I'm getting a linker error when I use CBusSystem::InvalidStopID directly in a test assertion. Why?"

**Original AI Response:**
"The issue is ODR (One Definition Rule). `InvalidStopID` is declared as `static const TStopID` with an in-class initializer in the header. While the compiler can use it as a constant expression in some contexts, passing it to EXPECT_EQ takes it by const reference, which ODR-uses it and requires an out-of-class definition in a translation unit. Since there's no .cpp file for CBusSystem (it's a pure abstract class), no such definition exists. The fix is to use `std::numeric_limits<CBusSystem::TStopID>::max()` in the test instead, which produces the same value without ODR-using the static member."

**Changes Made:** Replaced `CBusSystem::InvalidStopID` with `std::numeric_limits<CBusSystem::TStopID>::max()` in the out-of-bounds test and added `#include <limits>` at the top of the test file.

**Rationale:** AI explained the ODR issue clearly; using numeric_limits directly avoids the problem since it's a function call rather than an ODR-use of a static member.

---

### 13. Makefile Structure for Multiple Test Executables

**Prompt:** "How do I extend my existing Makefile to support new test executables for CCSVBusSystem and COpenStreetMap, where each needs different object file dependencies?"

**Original AI Response:**
"Add a new test executable target for each class following the same pattern as your existing targets. For CCSVBusSystem, link together the CSVBusSystem object, DSVReader object, StringDataSource, StringDataSink, and the test object file. For COpenStreetMap, link XMLReader, OpenStreetMap, StringDataSource, StringDataSink, and the test object, and add -lexpat since XMLReader depends on it. Add both new executables to your test target dependencies and add their run commands. The pattern rules for obj/%.o and testobj/%.o already handle compilation automatically, so you only need the new link targets."

**Changes Made:** Added testbin/testcsvbs and testbin/testosm as new link targets with their correct object dependencies. Linked -lexpat only for the OpenStreetMap test since XMLReader depends on Expat. Updated the test target dependency list and added run commands for both executables. Verified that the existing pattern rules automatically compiled new .cpp files without modification.

**Rationale:** ChatGPT clarified that the existing pattern rules already handled compilation, so only new link targets and dependency wiring were required. I verified object dependencies manually by tracing header includes and ensuring only the XML-based target required -lexpat.

---

### 14. Fixing Static Constant Linker Error

**Prompt:** ""Why am I getting an undefined symbol error for CStreetMap::InvalidNodeID during linking, even though it is declared in the header?"

**Original AI Response:**
"The error occurs because static const integral members require an out-of-class definition unless declared constexpr. Since InvalidNodeID and InvalidWayID are used in a way that triggers ODR-use, the linker expects a definition. Changing them to static constexpr ensures the compiler treats them as inline constants and eliminates the linker error."

**Changes Made:** Modified include/StreetMap.h to change static const to static constexpr for both InvalidNodeID and InvalidWayID.

**Rationale:** AI identified that the issue was due to ODR-use of static constants. Converting them to constexpr resolved the linker error without requiring separate definitions.

---

### 15. NImplementing COpenStreetMap Using PImpl Pattern

**Prompt:** "How should I structure COpenStreetMap to store nodes and ways efficiently while supporting both index-based and ID-based lookup?"

**Original AI Response:**
"Use a PImpl pattern with an internal SImplementation struct. Store nodes and ways in vectors to preserve insertion order, and use unordered_maps mapping IDs to vector indices for O(1) lookup by ID. Store attributes in both a vector (to preserve insertion order for GetAttributeKey) and an unordered_map (for fast HasAttribute/GetAttribute lookup)."

**Changes Made:** Implemented COpenStreetMap using a SImplementation struct containing vectors for nodes and ways and unordered_maps for ID lookups. Implemented attribute storage using both vector and unordered_map structures.

**Rationale:** AI explained structure ensured efficient lookup while satisfying the assignment requirement to preserve insertion order for attribute key indexing.

---

### 16. Parsing OSM XML with CXMLReader

**Prompt:** "How should I parse <node>, <way>, <tag>, and <nd> elements using CXMLReader to build the OpenStreetMap data structure?"

**Original AI Response:**
"Read XML entities in a loop using ReadEntity. When encountering a <node> or <way> start element, construct the corresponding object. For nodes, extract id, lat, lon, and parse child <tag> elements. For ways, extract id, parse child <nd ref> elements for node references, and parse child <tag> elements for attributes. Stop parsing when the corresponding end element is reached."

**Changes Made:** Implemented entity parsing in the constructor of COpenStreetMap. Added logic to detect StartElement and CompleteElement types, extract attributes, handle nested tags, and terminate parsing correctly at matching EndElement.

**Rationale:** The AI clarified how to correctly traverse XML entities using the provided reader abstraction rather than relying on raw XML parsing logic.

---

### 16. Writing GoogleTests for OpenStreetMap

**Prompt:** "What tests should I write to properly validate my COpenStreetMap implementation?"

**Original AI Response:**
"Write tests using in-memory XML strings via CStringDataSource. Validate NodeCount and WayCount, lookup by index and ID, attribute retrieval, missing attribute behavior, out-of-range index behavior, and correct node reference ordering in ways. Ensure sentinel values (nullptr and InvalidNodeID) are tested."

**Changes Made:** Created OpenStreetMapTest.cpp with multiple test cases verifying node counts, way counts, attribute handling, out-of-range access, ID lookups, and node reference ordering.

**Rationale:** AI identified required edge cases and sentinel behaviors that align with the interface specification and grading expectations.

---