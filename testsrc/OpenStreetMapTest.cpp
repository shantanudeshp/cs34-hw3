#include <gtest/gtest.h>
#include "OpenStreetMap.h"
#include "XMLReader.h"
#include "StringDataSource.h"

static std::shared_ptr<COpenStreetMap> BuildMapFromXML(const std::string &xml){
    auto DS = std::make_shared<CStringDataSource>(xml);
    auto Reader = std::make_shared<CXMLReader>(DS);
    return std::make_shared<COpenStreetMap>(Reader);
}

TEST(OpenStreetMap, CountsAndLookup){
    std::string XML =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
        "<osm version=\"0.6\">"
        "  <node id=\"1\" lat=\"38.500\" lon=\"-121.700\">"
        "    <tag k=\"name\" v=\"Stop A\"/>"
        "    <tag k=\"highway\" v=\"bus_stop\"/>"
        "  </node>"
        "  <node id=\"2\" lat=\"38.600\" lon=\"-121.800\"/>"
        "  <way id=\"10\">"
        "    <nd ref=\"1\"/>"
        "    <nd ref=\"2\"/>"
        "    <tag k=\"name\" v=\"Main St\"/>"
        "  </way>"
        "</osm>";

    auto Map = BuildMapFromXML(XML);
    ASSERT_NE(Map, nullptr);

    EXPECT_EQ(Map->NodeCount(), 2u);
    EXPECT_EQ(Map->WayCount(), 1u);

    // by index
    EXPECT_NE(Map->NodeByIndex(0), nullptr);
    EXPECT_NE(Map->NodeByIndex(1), nullptr);
    EXPECT_EQ(Map->NodeByIndex(2), nullptr);

    EXPECT_NE(Map->WayByIndex(0), nullptr);
    EXPECT_EQ(Map->WayByIndex(1), nullptr);

    // by id
    EXPECT_NE(Map->NodeByID(1), nullptr);
    EXPECT_NE(Map->NodeByID(2), nullptr);
    EXPECT_EQ(Map->NodeByID(999), nullptr);

    EXPECT_NE(Map->WayByID(10), nullptr);
    EXPECT_EQ(Map->WayByID(999), nullptr);
}

TEST(OpenStreetMap, NodeFieldsAndAttributes){
    std::string XML =
        "<osm>"
        "  <node id=\"5\" lat=\"38.123\" lon=\"-121.456\">"
        "    <tag k=\"name\" v=\"NodeName\"/>"
        "    <tag k=\"amenity\" v=\"cafe\"/>"
        "  </node>"
        "  <node id=\"6\" lat=\"38.000\" lon=\"-121.000\"/>"
        "</osm>";

    auto Map = BuildMapFromXML(XML);
    auto N5 = Map->NodeByID(5);
    ASSERT_NE(N5, nullptr);

    EXPECT_EQ(N5->ID(), 5u);
    auto Loc = N5->Location();
    EXPECT_NEAR(Loc.first, 38.123, 1e-9);
    EXPECT_NEAR(Loc.second, -121.456, 1e-9);

    EXPECT_EQ(N5->AttributeCount(), 2u);
    EXPECT_EQ(N5->GetAttributeKey(0), "name");
    EXPECT_EQ(N5->GetAttributeKey(1), "amenity");
    EXPECT_EQ(N5->GetAttributeKey(2), "");

    EXPECT_TRUE(N5->HasAttribute("name"));
    EXPECT_TRUE(N5->HasAttribute("amenity"));
    EXPECT_FALSE(N5->HasAttribute("missing"));

    EXPECT_EQ(N5->GetAttribute("name"), "NodeName");
    EXPECT_EQ(N5->GetAttribute("amenity"), "cafe");
    EXPECT_EQ(N5->GetAttribute("missing"), "");

    auto N6 = Map->NodeByID(6);
    ASSERT_NE(N6, nullptr);
    EXPECT_EQ(N6->AttributeCount(), 0u);
    EXPECT_EQ(N6->GetAttributeKey(0), "");
    EXPECT_FALSE(N6->HasAttribute("name"));
    EXPECT_EQ(N6->GetAttribute("name"), "");
}

TEST(OpenStreetMap, WayFieldsAndAttributes){
    std::string XML =
        "<osm>"
        "  <node id=\"1\" lat=\"0\" lon=\"0\"/>"
        "  <node id=\"2\" lat=\"0\" lon=\"0\"/>"
        "  <node id=\"3\" lat=\"0\" lon=\"0\"/>"
        "  <way id=\"99\">"
        "    <nd ref=\"1\"/>"
        "    <nd ref=\"2\"/>"
        "    <nd ref=\"3\"/>"
        "    <tag k=\"highway\" v=\"residential\"/>"
        "    <tag k=\"name\" v=\"Some Road\"/>"
        "  </way>"
        "</osm>";

    auto Map = BuildMapFromXML(XML);
    auto W = Map->WayByID(99);
    ASSERT_NE(W, nullptr);

    EXPECT_EQ(W->ID(), 99u);
    EXPECT_EQ(W->NodeCount(), 3u);
    EXPECT_EQ(W->GetNodeID(0), 1u);
    EXPECT_EQ(W->GetNodeID(1), 2u);
    EXPECT_EQ(W->GetNodeID(2), 3u);
    EXPECT_EQ(W->GetNodeID(3), CStreetMap::InvalidNodeID);

    EXPECT_EQ(W->AttributeCount(), 2u);
    EXPECT_EQ(W->GetAttributeKey(0), "highway");
    EXPECT_EQ(W->GetAttributeKey(1), "name");
    EXPECT_EQ(W->GetAttributeKey(2), "");

    EXPECT_TRUE(W->HasAttribute("highway"));
    EXPECT_TRUE(W->HasAttribute("name"));
    EXPECT_FALSE(W->HasAttribute("missing"));

    EXPECT_EQ(W->GetAttribute("highway"), "residential");
    EXPECT_EQ(W->GetAttribute("name"), "Some Road");
    EXPECT_EQ(W->GetAttribute("missing"), "");
}