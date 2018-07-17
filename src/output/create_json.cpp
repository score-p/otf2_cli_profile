/*
 This is part of the OTF-Profiler. Copyright by ZIH, TU Dresden 2016-2018.
 Authors: Maximillian Neumann, Denis Hünich, Jens Doleschal, Bill Williams
*/
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/pointer.h"
#include "rapidjson/prettywriter.h"
#include <iostream>
#include <fstream>
#include <string>
#include "all_data.h"

using namespace rapidjson;
using std::cout;
using std::string;


struct ProfileEntry {
  std::map<std::string, uint64_t> entries;
  template <typename Writer>
  void WriteProfile(Writer& w) const {
    w.StartObject();
    for(const auto& kv : entries) {
      w.Key(StringRef(kv.first.c_str()));
      w.Uint64(kv.second);
    }
    w.EndObject();
  }
};

struct WorkflowProfile {
  uint64_t job_id;
  uint32_t node_count;
  uint32_t process_count;
  uint32_t thread_count;
  std::map<std::string, uint64_t> counters;
  std::map<std::string, ProfileEntry> functions_by_paradigm;
  std::map<std::string, ProfileEntry> messages_by_paradigm;
  std::map<std::string, ProfileEntry> collops_by_paradigm;
  uint64_t parallel_region_time;
  uint64_t serial_time;
  uint64_t num_functions;
  uint64_t num_invocations;
  template <typename Writer>
  void WriteProfile(Writer& w) const;
  WorkflowProfile() :
    job_id(0),
    node_count(0),
    process_count(0),
    thread_count(0),
    parallel_region_time(0),
    serial_time(0),
    num_functions(0),
    num_invocations(0)
  {}
};

template <typename Map, typename Writer>
void WriteMapUnderKey(std::string key, const Map& m, Writer& w) {
  if(m.empty()) return;
  w.Key(StringRef(key.c_str()));
  w.StartObject();
  for(const auto& kv : m) {
    w.Key(StringRef(kv.first.c_str()));
    kv.second.WriteProfile(w);
  }
  w.EndObject();
}

template <typename Writer>
void WorkflowProfile::WriteProfile(Writer& w) const
{
  w.StartObject();
  w.Key("job id");
  w.Uint64(job_id);
  w.Key("node count");
  w.Uint(node_count);
  w.Key("process count");
  w.Uint(process_count);
  w.Key("thread count");
  w.Uint(thread_count);
  w.Key("hardware counters");
  w.StartArray();
  for(const auto& c : counters) {
    w.Key(StringRef(c.first.c_str()));
    w.Uint64(c.second);
  }
  w.EndArray();
  WriteMapUnderKey("functions", functions_by_paradigm, w);
  WriteMapUnderKey("messages", messages_by_paradigm, w);
  WriteMapUnderKey("collective operations", collops_by_paradigm, w);
  w.Key("parallel region time"); w.Uint64(parallel_region_time);
  w.Key("serial time"); w.Uint64(serial_time);
  w.Key("total functions"); w.Uint64(num_functions);
  w.Key("total calls"); w.Uint64(num_invocations);
  w.EndObject();
}


bool CreateJSON(AllData& alldata) {

  cout << "Creating JSON profile" << std::endl;
  WorkflowProfile profile;
  StringBuffer b;
  /*Pretty*/Writer<StringBuffer> w(b);
  for(const auto& n : alldata.definitions.system_tree) {
    switch(n.data.class_id) {
    case definitions::SystemClass::LOCATION:
      profile.thread_count++;
      break;
    case definitions::SystemClass::LOCATION_GROUP:
      profile.process_count++;
      break;
    case definitions::SystemClass::NODE:
      profile.node_count++;
      break;
    default:
      break;
    }
  }
  static std::string bytestr("bytes");
  static std::string timestr("time");
  static std::string countstr("count");
  
  for(const auto& call_node : alldata.call_path_tree) {
    const auto& r = alldata.definitions.regions.get(call_node.function_id);
    if(!r) {
      continue;
    }

    const auto& p = alldata.definitions.paradigms.get(r->paradigm_id);
    std::string paradigm = "Compiler";
    if(p) {
      paradigm = p->name;
    }

    profile.num_functions++;
    uint64_t excl_time = 0;
    
    for(const auto& one_node_data : call_node.node_data) {
      profile.num_invocations += one_node_data.second.f_data.count;
      auto& entries_map = profile.functions_by_paradigm[paradigm].entries;
      entries_map[countstr] += one_node_data.second.f_data.count;
      if(excl_time < one_node_data.second.f_data.excl_time) {
	excl_time = one_node_data.second.f_data.excl_time;
      }
      if(one_node_data.second.m_data.bytes_send)
	profile.messages_by_paradigm[paradigm].entries[bytestr] += one_node_data.second.m_data.bytes_send;
      if(one_node_data.second.m_data.bytes_recv)
	profile.messages_by_paradigm[paradigm].entries[bytestr] += one_node_data.second.m_data.bytes_recv;
      if(one_node_data.second.m_data.count_send)
	profile.messages_by_paradigm[paradigm].entries[countstr] += one_node_data.second.m_data.count_send;
      if(one_node_data.second.m_data.count_recv)
	 profile.messages_by_paradigm[paradigm].entries[countstr] += one_node_data.second.m_data.count_recv;
      if(one_node_data.second.c_data.bytes_send)
	profile.collops_by_paradigm[paradigm].entries[bytestr] += one_node_data.second.c_data.bytes_send;
      if(one_node_data.second.c_data.bytes_recv)
	profile.collops_by_paradigm[paradigm].entries[bytestr] += one_node_data.second.c_data.bytes_recv;
      if(one_node_data.second.c_data.count_send)
	profile.collops_by_paradigm[paradigm].entries[countstr] += one_node_data.second.c_data.count_send;
      if(one_node_data.second.c_data.count_recv)
	profile.collops_by_paradigm[paradigm].entries[countstr] += one_node_data.second.c_data.count_recv;
      for(const auto& metric : one_node_data.second.metrics) {
	if(metric.second.type == MetricDataType::UINT64) {
	  auto m = alldata.definitions.metrics.get(metric.first);
	  if(m) profile.counters[m->name] += (uint64_t)metric.second.data_excl;
	}
      }
    }
    if(call_node.node_data.size() == 1) {
      profile.serial_time += excl_time;
    } else {
      profile.parallel_region_time += excl_time;
    }
    profile.functions_by_paradigm[paradigm].entries[timestr] += excl_time;

  }
  profile.WriteProfile(w);
  string fname = alldata.params.output_file_prefix + ".json";
  std::ofstream outfile(fname.c_str());
  outfile << b.GetString() << std::endl;
  return true;
}
