#include <gtest/gtest.h>
#include "CSVBusSystem.h"
#include "DSVReader.h"
#include "StringDataSource.h"
#include <limits>

// Helper: construct a CCSVBusSystem from two in-memory CSV strings
static CCSVBusSystem MakeSystem(const std::string &stopsCSV, const std::string &routesCSV){
    return CCSVBusSystem(
        std::make_shared<CDSVReader>(std::make_shared<CStringDataSource>(stopsCSV), ','),
        std::make_shared<CDSVReader>(std::make_shared<CStringDataSource>(routesCSV), ',')
    );
}

// --- Empty input ---

TEST(CSVBusSystem, EmptyData){
    auto System = MakeSystem("stop_id,node_id\n", "route,stop_id\n");
    EXPECT_EQ(System.StopCount(), 0);
    EXPECT_EQ(System.RouteCount(), 0);
    EXPECT_EQ(System.StopByIndex(0), nullptr);
    EXPECT_EQ(System.RouteByIndex(0), nullptr);
}

// --- Stop count and basic lookup ---

TEST(CSVBusSystem, StopCount){
    auto System = MakeSystem(
        "stop_id,node_id\n1,100\n2,200\n3,300\n",
        "route,stop_id\n"
    );
    EXPECT_EQ(System.StopCount(), 3);
}

TEST(CSVBusSystem, StopByIndex){
    auto System = MakeSystem(
        "stop_id,node_id\n22043,2849810514\n22358,2849805223\n",
        "route,stop_id\n"
    );
    auto Stop = System.StopByIndex(0);
    ASSERT_NE(Stop, nullptr);
    EXPECT_EQ(Stop->ID(), 22043);
    EXPECT_EQ(Stop->NodeID(), 2849810514);
}

TEST(CSVBusSystem, StopByIndexOutOfBounds){
    auto System = MakeSystem("stop_id,node_id\n1,100\n", "route,stop_id\n");
    EXPECT_EQ(System.StopByIndex(1), nullptr);
    EXPECT_EQ(System.StopByIndex(9999), nullptr);
}

TEST(CSVBusSystem, StopByID){
    auto System = MakeSystem(
        "stop_id,node_id\n22043,2849810514\n22358,2849805223\n",
        "route,stop_id\n"
    );
    auto Stop = System.StopByID(22358);
    ASSERT_NE(Stop, nullptr);
    EXPECT_EQ(Stop->ID(), 22358);
    EXPECT_EQ(Stop->NodeID(), 2849805223);
}

TEST(CSVBusSystem, StopByIDNotFound){
    auto System = MakeSystem("stop_id,node_id\n1,100\n", "route,stop_id\n");
    EXPECT_EQ(System.StopByID(99999), nullptr);
}

// --- Route count and basic lookup ---

TEST(CSVBusSystem, RouteCount){
    auto System = MakeSystem(
        "stop_id,node_id\n1,100\n2,200\n3,300\n",
        "route,stop_id\nA,1\nA,2\nB,3\n"
    );
    EXPECT_EQ(System.RouteCount(), 2);
}

TEST(CSVBusSystem, RouteByIndex){
    auto System = MakeSystem(
        "stop_id,node_id\n1,100\n2,200\n",
        "route,stop_id\nA,1\nA,2\n"
    );
    auto Route = System.RouteByIndex(0);
    ASSERT_NE(Route, nullptr);
    EXPECT_EQ(Route->Name(), "A");
    EXPECT_EQ(Route->StopCount(), 2);
}

TEST(CSVBusSystem, RouteByIndexOutOfBounds){
    auto System = MakeSystem("stop_id,node_id\n1,100\n", "route,stop_id\nA,1\n");
    EXPECT_EQ(System.RouteByIndex(1), nullptr);
    EXPECT_EQ(System.RouteByIndex(9999), nullptr);
}

TEST(CSVBusSystem, RouteByName){
    auto System = MakeSystem(
        "stop_id,node_id\n1,100\n2,200\n3,300\n",
        "route,stop_id\nA,1\nA,2\nB,3\n"
    );
    auto RouteA = System.RouteByName("A");
    ASSERT_NE(RouteA, nullptr);
    EXPECT_EQ(RouteA->Name(), "A");
    EXPECT_EQ(RouteA->StopCount(), 2);

    auto RouteB = System.RouteByName("B");
    ASSERT_NE(RouteB, nullptr);
    EXPECT_EQ(RouteB->StopCount(), 1);
}

TEST(CSVBusSystem, RouteByNameNotFound){
    auto System = MakeSystem("stop_id,node_id\n1,100\n", "route,stop_id\nA,1\n");
    EXPECT_EQ(System.RouteByName("DOESNOTEXIST"), nullptr);
}

// --- Route stop ordering ---

TEST(CSVBusSystem, RouteStopOrder){
    // Stops on route R1 are listed as 1, 3, 2 — insertion order must be preserved
    auto System = MakeSystem(
        "stop_id,node_id\n1,100\n2,200\n3,300\n",
        "route,stop_id\nR1,1\nR1,3\nR1,2\n"
    );
    auto Route = System.RouteByName("R1");
    ASSERT_NE(Route, nullptr);
    EXPECT_EQ(Route->StopCount(), 3);
    EXPECT_EQ(Route->GetStopID(0), 1);
    EXPECT_EQ(Route->GetStopID(1), 3);
    EXPECT_EQ(Route->GetStopID(2), 2);
}

TEST(CSVBusSystem, GetStopIDOutOfBounds){
    auto System = MakeSystem(
        "stop_id,node_id\n1,100\n",
        "route,stop_id\nR1,1\n"
    );
    auto Route = System.RouteByName("R1");
    ASSERT_NE(Route, nullptr);
    EXPECT_EQ(Route->GetStopID(999), std::numeric_limits<CBusSystem::TStopID>::max());
}

// --- Multiple routes sharing stops ---

TEST(CSVBusSystem, MultipleRoutes){
    auto System = MakeSystem(
        "stop_id,node_id\n10,1000\n20,2000\n30,3000\n",
        "route,stop_id\nX,10\nX,20\nY,20\nY,30\n"
    );
    EXPECT_EQ(System.RouteCount(), 2);
    auto RouteX = System.RouteByName("X");
    auto RouteY = System.RouteByName("Y");
    ASSERT_NE(RouteX, nullptr);
    ASSERT_NE(RouteY, nullptr);
    EXPECT_EQ(RouteX->StopCount(), 2);
    EXPECT_EQ(RouteY->StopCount(), 2);
    EXPECT_EQ(RouteX->GetStopID(0), 10);
    EXPECT_EQ(RouteY->GetStopID(0), 20);
}

// --- Large IDs (OSM node IDs exceed 32-bit range) ---

TEST(CSVBusSystem, LargeNodeID){
    auto System = MakeSystem(
        "stop_id,node_id\n22043,2849810514\n",
        "route,stop_id\n"
    );
    auto Stop = System.StopByID(22043);
    ASSERT_NE(Stop, nullptr);
    // 2849810514 exceeds 32-bit unsigned max (4294967295 is fine, but tests 64-bit parsing)
    EXPECT_EQ(Stop->NodeID(), 2849810514ULL);
}
