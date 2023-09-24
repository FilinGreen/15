#pragma once

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <vector>
#include <utility>
#include <sstream>
#include <memory>
#include <optional>

#include "geo.h"
#include "transport_catalogue.h"
#include "json.h"
#include "svg.h"
#include "map_renderer.h"
#include "json_builder.h"
#include "ranges.h"
#include "graph.h"
#include "router.h"
#include "transport_router.h"
#include "serialization.h"

class JSONReader {
public:
	explicit JSONReader(const json::Node& data);
	JSONReader() = default;

	
	void LoadData();
	void ProcessQuery(std::ostream& out);

	void Serialization();
	void Deserialization();

private:
	const json::Node& data_;
	TransportCatalogue catalog_;

	json::Node base_data_;
	json::Node setting_data_;
	json::Node serialization_data_;
	renderer::RenderSettings settings_;

	int bus_wait_time_ = 0;
	double bus_velocity_ = 0;

	//--------------------private functions
	void LoadSettings();
	void LoadStops();
	void LoadBuses();
	
	void ProcessBus(json::Builder& builder, const json::Node& request);
	void ProcessStop(json::Builder& builder, const json::Node& request);
	void ProcessRoute(json::Builder& builder, const json::Node& request, TransportRouter& transport_router);
	void ProcessMap(json::Builder& builder, const json::Node& request);

	renderer::RenderSettings LoadRenderSettings();
	renderer::SphereProjector SetProjector();
	svg::Color LoadColor(const json::Node& data);
	svg::Point LoadOffset(const std::vector<json::Node>& data);
};



