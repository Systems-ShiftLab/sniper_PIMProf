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

void DataReuse::UpdateTrie(TrieNode *root, DataReuseSegment &seg)
{
   // A reuse chain segment of size 1 can be removed
   if (seg.size() <= 1)
      return;

   // seg.print(std::cout);

   TrieNode *curNode = root;
   std::set<BBLID>::iterator it = seg._set.begin();
   std::set<BBLID>::iterator eit = seg._set.end();
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
   TrieNode *temp = curNode->_children[seg._headID];
   if (temp == NULL)
   {
      temp = new TrieNode();
      temp->_parent = curNode;
      temp->_curID = seg._headID;
      curNode->_children[seg._headID] = temp;
      _leaves.push_back(temp);
   }
   temp->_isLeaf = true;
   temp->_count += seg.getCount();
}

void DataReuse::ExportSegment(DataReuseSegment &seg, TrieNode *leaf)
{
   assert(leaf->_isLeaf);
   seg.setHead(leaf->_curID);
   seg.setCount(leaf->_count);

   TrieNode *temp = leaf;
   while (temp->_parent != NULL)
   {
      seg.insert(temp->_curID);
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

void DataReuse::PrintTrie(std::ostream &out, TrieNode *root, int parent, int &count)
{
   if (root->_isLeaf)
   {
      out << "    V_" << count << " [shape=box, label=\"head = " << root->_curID << "\n cnt = " << root->_count << "\"];" << std::endl;
      out << "    V_" << parent << " -> V_" << count << ";" << std::endl;
      parent = count;
      count++;
   }
   else
   {
      out << "    V_" << count << " [label=\"" << root->_curID << "\"];" << std::endl;
      out << "    V_" << parent << " -> V_" << count << ";" << std::endl;
      parent = count;
      count++;
      std::map<BBLID, TrieNode *>::iterator it = root->_children.begin();
      std::map<BBLID, TrieNode *>::iterator eit = root->_children.end();
      for (; it != eit; it++)
      {
         DataReuse::PrintTrie(out, it->second, parent, count);
      }
   }
}

std::ostream &DataReuse::print(std::ostream &out, TrieNode *root)
{
   int parent = 0;
   int count = 1;
   out << "digraph trie {" << std::endl;
   std::map<BBLID, TrieNode *>::iterator it = root->_children.begin();
   std::map<BBLID, TrieNode *>::iterator eit = root->_children.end();
   out << "    V_0"
       << " [label=\"root\"];" << std::endl;
   for (; it != eit; it++)
   {
      DataReuse::PrintTrie(out, it->second, parent, count);
   }
   out << "}" << std::endl;
   return out;
}

void PIMProfThreadStats::PIMProfBBLStart(uint64_t hi, uint64_t lo)
{
   UUID bblhash = UUID(hi, lo);
   auto it = m_bblhash2bblid.find(bblhash);
   BBLID bblid = GLOBAL_BBLID;
   if (it == m_bblhash2bblid.end()) {
      bblid = m_bblhash2bblid.size();
      m_bblhash2bblid.insert(std::make_pair(bblhash, bblid));
      printf("%lu %lu %lx %lx\n", bblid, m_bblid2stats.size(), bblhash.first, bblhash.second);
      auto begin = m_bblid2stats.begin();
      auto end = m_bblid2stats.end();
      printf("%p %p %p\n", &m_bblid2stats, &*begin, &*end);
      PIMProfBBLStats temp;
      temp.bblid = bblid; temp.bblhash = bblhash;
      printf("%lu %lu %lx %lx\n", temp.bblid, m_bblid2stats.size(), temp.bblhash.first, temp.bblhash.second);
      
      m_bblid2stats.push_back(temp);
      printf("wow\n");
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
   m_bblid2stats[bblid].elapsed_time += time;
   m_bblid2stats[bblid].instruction_count += instr;
}

void PIMProfThreadStats::PIMProfAddMemory(uint64_t memory_access)
{
   BBLID bblid = m_current_bblid.back();
   m_bblid2stats[bblid].memory_access += memory_access;
}

void PIMProfThreadStats::PIMProfAddOffloadingTime(uint64_t time)
{
   m_pim_time += time;
}

void PIMProfThreadStats::PIMProfDumpStats(std::ostream &ofs)
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
      << m_bblid2stats[bblid].elapsed_time << " " << m_bblid2stats[bblid].instruction_count << " " << m_bblid2stats[bblid].memory_access << std::endl;
   }
}

void PIMProfThreadStats::PIMProfInsertSegOnHit(uint64_t tag, int placeholder)
{
   // BBLID bblid = 
   // PIMProf::DataReuseSegment &seg = m_tag_seg_map[tag];
   // seg.insert(bblid);
   // // int32_t threadcount = _storage->_cost_package->_thread_count;
   // // if (threadcount > seg.getCount())
   // seg.setCount(1);
   // // split then insert on store
   // if (mem_op_type == Core::mem_op_t::WRITE) {
   //    m_data_reuse.UpdateTrie(m_data_reuse.getRoot(), seg);
   //    seg.clear();
   //    seg.insert(bblid);
   //    // if (threadcount > seg.getCount())
   //    seg.setCount(1);
   // }
}
void PIMProfThreadStats::PIMProfSplitSegOnMiss(uint64_t tag)
{
   // PIMProf::DataReuseSegment &seg = m_tag_seg_map[tag];
   // m_data_reuse.UpdateTrie(m_data_reuse.getRoot(), seg);
   // seg.clear();
}
