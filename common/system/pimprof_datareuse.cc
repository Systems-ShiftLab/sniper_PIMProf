//===- [Yizhou]                                      ------------*- C++ -*-===//
//
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//
#include <cassert>

#include "pimprof_datareuse.h"

using namespace PIMProf;

/* ===================================================================== */
/* DataReuse */
/* ===================================================================== */

DataReuse::DataReuse()
{
   _root = new TrieNode();
}

DataReuse::~DataReuse()
{
   DeleteTrie(_root);
}

void DataReuse::UpdateTrie(TrieNode *root, const DataReuseSegment *seg)
{
   // A reuse chain segment of size 1 can be removed
   if (seg->size() <= 1)
      return;

   // seg->print(std::cout);

   TrieNode *curNode = root;
   std::set<BBLID>::iterator it = seg->_set.begin();
   std::set<BBLID>::iterator eit = seg->_set.end();
   for (; it != eit; it++)
   {
      BBLID curID = *it;
      TrieNode *temp = curNode->_children[curID];
      if (temp == NULL)
      {
         temp = new TrieNode();
         temp->_parent = curNode;
         temp->_curID = curID;
         curNode->_children[curID] = temp;
      }
      curNode = temp;
   }
   TrieNode *temp = curNode->_children[seg->_headID];
   if (temp == NULL)
   {
      temp = new TrieNode();
      temp->_parent = curNode;
      temp->_curID = seg->_headID;
      curNode->_children[seg->_headID] = temp;
      _leaves.push_back(temp);
   }
   temp->_isLeaf = true;
   temp->_count += seg->getCount();
}

void DataReuse::ExportSegment(DataReuseSegment *seg, TrieNode *leaf)
{
   assert(leaf->_isLeaf);
   seg->setHead(leaf->_curID);
   seg->setCount(leaf->_count);

   TrieNode *temp = leaf;
   while (temp->_parent != NULL)
   {
      seg->insert(temp->_curID);
      temp = temp->_parent;
   }
}

void DataReuse::DeleteTrie(TrieNode *root)
{
   if (!root->_isLeaf)
   {
      std::map<BBLID, TrieNode *>::iterator it = root->_children.begin();
      std::map<BBLID, TrieNode *>::iterator eit = root->_children.end();
      for (; it != eit; it++)
      {
         DataReuse::DeleteTrie(it->second);
      }
   }
   delete root;
}

void DataReuse::PrintDotGraphHelper(std::ostream &out, TrieNode *root, int parent, int &count)
{
   int64_t curID = root->_curID;
   if (root->_isLeaf)
   {
      out << "    V_" << count << " [shape=box, label=\"head = " << curID << "\n cnt = " << root->_count << "\"];" << std::endl;
      out << "    V_" << parent << " -> V_" << count << ";" << std::endl;
      parent = count;
      count++;
   }
   else
   {
      out << "    V_" << count << " [label=\"" << curID << "\"];" << std::endl;
      out << "    V_" << parent << " -> V_" << count << ";" << std::endl;
      parent = count;
      count++;
      auto it = root->_children.begin();
      auto eit = root->_children.end();
      for (; it != eit; it++)
      {
         DataReuse::PrintDotGraphHelper(out, it->second, parent, count);
      }
   }
}

std::ostream &DataReuse::PrintDotGraph(std::ostream &out)
{
   int parent = 0;
   int count = 1;
   out << "digraph trie {" << std::endl;
   auto it = _root->_children.begin();
   auto eit = _root->_children.end();
   out << "    V_0"
       << " [label=\"root\"];" << std::endl;
   for (; it != eit; it++)
   {
      DataReuse::PrintDotGraphHelper(out, it->second, parent, count);
   }
   out << "}" << std::endl;
   return out;
}

std::ostream &DataReuse::PrintAllSegments(std::ostream &out)
{
   for (auto it = _leaves.begin(); it != _leaves.end(); it++) {
      DataReuseSegment seg;
      ExportSegment(&seg, *it);
      seg.print(out);
   }
   return out;
}

/* ===================================================================== */
/* PIMPRofThreadStats */
/* ===================================================================== */

void PIMProfThreadStats::PIMProfBBLStart(uint64_t hi, uint64_t lo)
{
   UUID bblhash = UUID(hi, lo);
   auto it = m_bblhash2bblid.find(bblhash);
   BBLID bblid = GLOBAL_BBLID;
   if (it == m_bblhash2bblid.end()) {
      bblid = m_bblhash2bblid.size();
      m_bblhash2bblid.insert(std::make_pair(bblhash, bblid));

      m_bblid2stats.push_back(new PIMProfBBLStats(bblid, bblhash));
      if (bblhash.first <= 0x10000000) {
         printf("%lu %lu %lx %lx\n", bblid, m_bblid2stats.size(), bblhash.first, bblhash.second);
      }
   }
   else {
      bblid = it->second;
   }
   m_current_bblid.push_back(bblid);
}

void PIMProfThreadStats::PIMProfBBLEnd(uint64_t hi, uint64_t lo)
{
   UUID bblhash = UUID(hi, lo);
   auto it = m_bblhash2bblid.find(bblhash);
   assert(it != m_bblhash2bblid.end());
   assert(m_current_bblid.back() == it->second);
   m_current_bblid.pop_back();
}

void PIMProfThreadStats::PIMProfOffloadStart(uint64_t hi, uint64_t type)
{
   m_using_pim = true;
   // type is used to distinguish actual BBL start and end
   // since the start of a BBL could be the end of offloading
   // our compiling tool only provide the high bits of bblhash in this case
   if (type == 0) {
      PIMProfBBLStart(hi, 0);
   }
   else {
      PIMProfBBLEnd(hi, 0);
   }
}

void PIMProfThreadStats::PIMProfOffloadEnd(uint64_t hi, uint64_t type)
{
   m_using_pim = false;
   if (type == 0) {
      PIMProfBBLStart(hi, 0);
   }
   else {
      PIMProfBBLEnd(hi, 0);
   }
}

void PIMProfThreadStats::PIMProfAddTimeInstruction(uint64_t time, uint64_t instr)
{
   BBLID bblid = m_current_bblid.back();
   PIMProfGetBBLStats(bblid)->elapsed_time += time;
   PIMProfGetBBLStats(bblid)->instruction_count += instr;
}

void PIMProfThreadStats::PIMProfAddMemory(uint64_t memory_access)
{
   BBLID bblid = m_current_bblid.back();
   PIMProfGetBBLStats(bblid)->memory_access += memory_access;
}

void PIMProfThreadStats::PIMProfAddOffloadingTime(uint64_t time)
{
   m_pim_time += time;
}

void PIMProfThreadStats::PIMProfInsertSegOnHit(uintptr_t tag, Core::mem_op_t mem_op_type)
{
   BBLID bblid = m_current_bblid.back();
   DataReuseSegment *seg;
   auto it = m_tag2seg.find(tag);
   if (it == m_tag2seg.end()) {
      seg = new DataReuseSegment();
      m_tag2seg.insert(std::make_pair(tag, seg));
   }
   else {
      seg = it->second;
   }
  
   seg->insert(bblid);
   // int32_t threadcount = _storage->_cost_package->_thread_count;
   // if (threadcount > seg->getCount())
   seg->setCount(1);
   // split then insert on store
   if (mem_op_type == Core::mem_op_t::WRITE) {
      m_data_reuse->UpdateTrie(m_data_reuse->getRoot(), seg);
      seg->clear();
      seg->insert(bblid);
      // if (threadcount > seg->getCount())
      seg->setCount(1);
   }
}
void PIMProfThreadStats::PIMProfSplitSegOnMiss(uintptr_t tag)
{
   DataReuseSegment *seg;
   auto it = m_tag2seg.find(tag);
   if (it == m_tag2seg.end()) return; // ignore it if there is no existing segment
   seg = it->second;
   m_data_reuse->UpdateTrie(m_data_reuse->getRoot(), seg);
   seg->clear();
}

void PIMProfThreadStats::PIMProfPrintStats(std::ostream &ofs)
{
   std::vector<std::pair<UUID, BBLID>> m_bblhash_sorted(m_bblhash2bblid.begin(), m_bblhash2bblid.end());
   std::sort(
      m_bblhash_sorted.begin(),
      m_bblhash_sorted.end(),
      [](std::pair<UUID, BBLID> &a, std::pair<UUID, BBLID> &b) { return a.first.first < b.first.first; }
   );
   for (auto it = m_bblhash_sorted.begin(); it != m_bblhash_sorted.end(); ++it) {
      UUID bblhash = it->first;
      BBLID bblid = it->second;
      ofs << tid << " "
      << std::hex << bblhash.first << " " << bblhash.second << " " << std::dec
      << PIMProfGetBBLStats(bblid)->elapsed_time << " " << PIMProfGetBBLStats(bblid)->instruction_count << " " << PIMProfGetBBLStats(bblid)->memory_access << std::endl;
   }
   ofs << tid << " " << std::hex << m_globalbblstats->bblhash.first << m_globalbblstats->bblhash.second << " " << std::dec << m_globalbblstats->elapsed_time << " " << m_globalbblstats->instruction_count << " " << m_globalbblstats->memory_access << std::endl;
}

void PIMProfThreadStats::PIMProfPrintPIMTime(std::ostream &ofs)
{
   ofs << m_pim_time << std::endl;
}

void PIMProfThreadStats::PIMProfPrintDataReuseDotGraph(std::ostream &ofs)
{
   m_data_reuse->PrintDotGraph(ofs);
}

void PIMProfThreadStats::PIMProfPrintDataReuseSegments(std::ostream &ofs)
{
   m_data_reuse->PrintAllSegments(ofs);
}
