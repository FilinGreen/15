#pragma once

#include <fstream>
#include <ostream>
#include <string>

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <graph.pb.h>
#include <transport_router.pb.h>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

namespace srlz{

	void Serialization(TransportCatalogue& catalog, TransportRouter& transport_router, renderer::RenderSettings& settings, std::string out);
	void Deserialization(TransportCatalogue& catalog, renderer::RenderSettings& settings, int& bus_wait_time, double& bus_velocity, std::string in);


	Protobuf::TransportData ConvertCatalog(TransportCatalogue& catalog);
	Protobuf::RenderSettings ConvertSettings(TransportCatalogue& catalog, renderer::RenderSettings& settings);
	Protobuf::Color ConvertColor(svg::Color color);
	Protobuf::GraphEdges ConvertEdges(TransportRouter& transport_router);
	Protobuf::Edge ConvertEdge(const graph::Edge<double>& edge);
	Protobuf::RouterSettings ConvertRouterSettings(TransportRouter& transport_router);

	TransportCatalogue ConvertCatalogFromProto(Protobuf::TransportData& proto_catalog);
	renderer::RenderSettings ConvertSettingsFromProto(Protobuf::RenderSettings proto_settings);
	svg::Color ConvertColorFromProto(Protobuf::Color proto_color);
	std::vector<graph::Edge<double>> ConvertEdgesFromProto(Protobuf::GraphEdges& proto_edges);
	graph::Edge<double> ConvertEdgeFromProto(const Protobuf::Edge& proto_edge);

}//namespace srlz
