#pragma once

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <optional>

#include "ranges.h"
#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"
#include "json_builder.h"


struct EdgeInfo {
	EdgeInfo(std::string& name_, int stopcount_, size_t stopid_) :name(name_), stop_count(stopcount_), stopid(stopid_) {}

	std::string name;
	int stop_count = 0;
	size_t stopid = 0;
};

class TransportRouter {
public:
	explicit TransportRouter(TransportCatalogue& catalog, int bt, double bv);
	

	void ProcessRoute(json::Builder& builder, const json::Node& request, size_t from, size_t to);

	std::vector<graph::Edge<double>>& GetEdges();
	int GetWait();
	double GetVelocity();


private:
	TransportCatalogue& catalog_;
	std::unique_ptr <graph::DirectedWeightedGraph< double >> graph_;
	std::unique_ptr <graph::Router <double>> router_;

	std::map <graph::EdgeId, std::unique_ptr<EdgeInfo>> edge_info_;
	int bus_wait_time_ = 0;
	double bus_velocity_ = 0;

	//------------------------private functions
	void FillGraph();
	void FillGraphFromProto(std::vector<graph::Edge<double>>& edges);
	void AddBusEdges(Bus& bus, std::vector<Stop* >::iterator begin, std::vector<Stop* >::iterator end);
	void AddCircleBusEdges(Bus& bus);
	void AddLineBusEdges(Bus& bus);


};