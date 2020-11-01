//===- [Yizhou]                                      ------------*- C++ -*-===//
//
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//
#ifndef __DATAREUSE_H__
#define __DATAREUSE_H__

#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <unistd.h>
#include <cmath>
#include <stack>
#include <list>
#include <set>
#include <map>
#include <algorithm>
#include <unordered_map>

namespace PIMProf
{

/* ===================================================================== */
/* fast vector */
/* ===================================================================== */
template <class T>
class FastVector {
public:
	FastVector();
	~FastVector();
	void push_back(T);
	//void insert(int pos, FastVector<T> &inputFV, int begin, int end);
	void append(FastVector<T> &);
	void append(T *, size_t);
	int size();
	void clear();
	T& operator[](int idx) const { return arr_vector[idx]; }
private:
	T* arr_vector;
	int cur_size;
	int vector_max_size;
};


template <class T>
FastVector<T>::FastVector()
{
	cur_size = 0;
	vector_max_size = 200;
	arr_vector = (T *) malloc(vector_max_size * sizeof(T));
	//printf("%lu\n", vector_max_size * sizeof(T));
}

template <class T>
FastVector<T>::~FastVector()
{
	free(arr_vector);
}

template <class T>
void FastVector<T>::push_back(T input)
{
	if (cur_size >= vector_max_size) {
		vector_max_size *= 10;
		arr_vector = (T *) realloc(arr_vector, vector_max_size * sizeof(T));
		//printf("@@@@ push_back reallocate\n");
	}
	arr_vector[cur_size] = input;
	++cur_size;
}

template <class T>
void FastVector<T>::append(FastVector<T> &input)
{
	if (input.size() + cur_size >= vector_max_size) {
		vector_max_size = (cur_size + input.size()) * 10;
		arr_vector = (T *) realloc(arr_vector, vector_max_size * sizeof(T));
		//printf("@@@@ append reallocate\n");
	}

	memcpy(arr_vector + cur_size, input.arr_vector, input.size() * sizeof(T));
	cur_size += input.size();
}


template <class T>
void FastVector<T>::append(T *input, size_t input_size)
{
	if (input_size + cur_size >= vector_max_size) {
		vector_max_size = (cur_size + input_size) * 10;
		arr_vector = (T *) realloc(arr_vector, vector_max_size * sizeof(T));
		//printf("@@@@ append reallocate\n");
	}

	memcpy(arr_vector + cur_size, input, input_size * sizeof(T));

	cur_size += input_size;
}

/*
template <class T>
void FastVector<T>::insert(int pos, FastVector<T> &inputFV, int begin, int end)
{
	//TODO: memcpy the covered part to the end
	if (end - begin + 1 + cur_size >= vector_max_size) {
		vector_max_size = (cur_size + end - begin + 1) * 10;
		arr_vector = (T*) realloc(arr_vector, vector_max_size * sizeof(T));
	}
	memcpy(arr_vector + cur_size, arr_vector + pos + 1, end - begin + 1);
	memcpy(arr_vector + pos, inputFV.arr_vector + begin, end - begin + 1);
}
*/

template <class T>
int FastVector<T>::size()
{
	return cur_size;
}


template <class T>
void FastVector<T>::clear()
{
	cur_size = 0;
	vector_max_size = 200;
	arr_vector = (T *) realloc(arr_vector, vector_max_size * sizeof(T));
}


/* ===================================================================== */
/* Typedefs and constants */
/* ===================================================================== */
const uint64_t GLOBAL_BBLID = -1;

typedef uint32_t CACHE_STATS;
typedef double COST;
typedef uint64_t BBLID;
typedef std::pair<uint64_t, uint64_t> UUID;

enum CostSite
{
   CPU,
   PIM,
   MAX_COST_SITE,
   INVALID = 0x3fffffff // a placeholder that does not count as a cost site
};

enum ACCESS_TYPE
{
   ACCESS_TYPE_LOAD,
   ACCESS_TYPE_STORE,
   ACCESS_TYPE_NUM
};

/* ===================================================================== */
/* DataReuseSegment */
/* ===================================================================== */
typedef std::pair<BBLID, ACCESS_TYPE> BBLOP;

class DataReuseSegment
{
   friend class DataReuse;

private:
   BBLID _headID;
   std::set<BBLID> _set;
   int _count;

public:
   inline DataReuseSegment()
   {
      _headID = -1;
      _count = 1;
   }

   inline size_t size() const
   {
      return _set.size();
   }

   inline void insert(BBLID bblid)
   {
      if (_set.empty())
         _headID = bblid;
      _set.insert(bblid);
   }

   inline void insert(DataReuseSegment &seg)
   {
      _set.insert(seg._set.begin(), seg._set.end());
   }

   inline std::vector<BBLID> diff(DataReuseSegment &seg)
   {
      std::vector<BBLID> result;
      std::set_difference(
          _set.begin(), _set.end(),
          seg.begin(), seg.end(),
          std::inserter(result, result.end()));
      return result;
   }

   inline void clear()
   {
      _headID = -1;
      _set.clear();
      _count = 1;
   }

   inline std::set<BBLID>::iterator begin()
   {
      return _set.begin();
   }

   inline std::set<BBLID>::iterator end()
   {
      return _set.end();
   }

   inline void setHead(BBLID head)
   {
      _headID = head;
   }

   inline BBLID getHead() const
   {
      return _headID;
   }

   inline void setCount(int count)
   {
      _count = count;
   }

   inline int getCount() const
   {
      return _count;
   }

   inline bool operator==(DataReuseSegment &rhs)
   {
      return (_headID == rhs._headID && _set == rhs._set);
   }

   inline std::ostream &print(std::ostream &out)
   {
      out << "{ ";
      out << _headID << " | ";
      for (auto it = _set.begin(); it != _set.end(); it++)
      {
         out << *it << ", ";
      }
      out << "}";
      out << std::endl;
      return out;
   }
};

/* ===================================================================== */
/* TrieNode */
/* ===================================================================== */
class TrieNode
{
public:
   // the leaf node stores the head of the segment
   bool _isLeaf;
   std::map<BBLID, TrieNode *> _children;
   BBLID _curID;
   TrieNode *_parent;
   int64_t _count;

public:
   inline TrieNode()
   {
      _isLeaf = false;
      _parent = NULL;
      _count = 0;
   }
};

/* ===================================================================== */
/* DataReuse */
/* ===================================================================== */
/// We split a reuse chain into multiple segments that starts
/// with a W and ends with a W, for example:
/// A reuse chain: R W R R R R W R W W R R W R
/// can be splitted into: R W | R R R R W | R W | W | R R W | R
/// this is stored as segments that starts with a W and ends with a W:
/// R W; W R R R R W; W R W; W W; W R R W; W R

/// If all BB in a segment are executed in the same place,
/// then there is no reuse cost;
/// If the initial W is on PIM and there are subsequent R/W on CPU,
/// then this segment contributes to a flush of PIM and data fetch from CPU;
/// If the initial W is on CPU and there are subsequent R/W on PIM,
/// then this segment contributes to a flush of CPU and data fetch from PIM.
class DataReuse
{
private:
   TrieNode *_root;
   std::vector<TrieNode *> _leaves;

public:
   DataReuse();
   ~DataReuse();

public:
   void UpdateTrie(TrieNode *root, DataReuseSegment &seg);
   void DeleteTrie(TrieNode *root);
   void ExportSegment(DataReuseSegment &seg, TrieNode *leaf);
   void PrintTrie(std::ostream &out, TrieNode *root, int parent, int &count);
   std::ostream &print(std::ostream &out, TrieNode *root);

   inline TrieNode *getRoot()
   {
      return _root;
   }

   inline std::vector<TrieNode *> &getLeaves()
   {
      return _leaves;
   }
};

/* ===================================================================== */
/* PIMProf Thread Data Collection */
/* ===================================================================== */

class PIMProfHashFunc
{
public:
   // assuming UUID is already murmurhash-ed.
   std::size_t operator()(const UUID &key) const
   {
      size_t result = key.first ^ key.second;
      return result;
   }
};

class PIMProfBBLStats
{
public:
   BBLID bblid;
   UUID bblhash;
   uint64_t elapsed_time; // in nanoseconds
   uint64_t instruction_count;
   uint64_t memory_access;
   // int temp[8];

   PIMProfBBLStats(
      BBLID _bblid = 0,
      UUID _bblhash = UUID(0, 0),
      uint64_t _elapsed_time = 0,
      uint64_t _instruction_count = 0,
      uint64_t _memory_access = 0)
      : bblid(_bblid)
      , bblhash(_bblhash)
      , elapsed_time(_elapsed_time)
      , instruction_count(_instruction_count)
      , memory_access(_memory_access)
   {
   }
};

class PIMProfThreadStats
{
private:
   int tid;
   bool m_using_pim;
   std::vector<BBLID> m_current_bblid;
   uint64_t m_pim_time;

   // store the nanosecond count of each basic block
   std::unordered_map<UUID, BBLID, PIMProfHashFunc> m_bblhash2bblid;
   std::vector<PIMProfBBLStats *> m_bblid2stats;
   PIMProfBBLStats *m_globalbblstats;

public:
   PIMProfThreadStats(int _tid = 0)
   : tid(_tid)
   {
      m_using_pim = false;
      m_pim_time = 0;
      // UUID(GLOBAL_BBLID, GLOBAL_BBLID) is the region outside main function.
      m_current_bblid.push_back(GLOBAL_BBLID);
      // UUID(0, 0) is the region that is inside main function but outside
      // any other BBL, we assign this region as BBL 0.
      m_bblhash2bblid.insert(std::make_pair(UUID(0, 0), 0));
      m_bblid2stats.push_back(new PIMProfBBLStats(0, UUID(0, 0)));
      m_globalbblstats = new PIMProfBBLStats(GLOBAL_BBLID, UUID(GLOBAL_BBLID, GLOBAL_BBLID));
   }

   PIMProfThreadStats()
   {
      for (auto it = m_bblid2stats.begin(); it != m_bblid2stats.end(); ++it) {
         delete *it;
      }
      delete m_globalbblstats;
   }

   void setTid(int _tid) {
      tid = _tid;
   }

   bool PIMProfIsUsingPIM() { return m_using_pim; }
   
   int64_t PIMProfGetCurrentBBLID() { return m_current_bblid.back(); }

   PIMProfBBLStats *PIMProfGetBBLStats(BBLID bblid)
   {
      return (bblid == GLOBAL_BBLID ?
         m_globalbblstats : m_bblid2stats[bblid]);
   }

   UUID PIMProfGetCurrentBBLHash()
   {
      BBLID bblid = m_current_bblid.back();
      return PIMProfGetBBLStats(bblid)->bblhash;
   }

   void PIMProfBBLStart(uint64_t hi, uint64_t lo);
   void PIMProfBBLEnd(uint64_t hi, uint64_t lo);

   void PIMProfOffloadStart(uint64_t hi, uint64_t type);
   void PIMProfOffloadEnd(uint64_t hi, uint64_t type);

   void PIMProfAddTimeInstruction(uint64_t time, uint64_t instr);
   void PIMProfAddMemory(uint64_t memory_access);

   void PIMProfAddOffloadingTime(uint64_t time);

   void PIMProfDumpStats(std::ostream &ofs);

   void PIMProfInsertSegOnHit(uint64_t tag, int placeholder);
   void PIMProfSplitSegOnMiss(uint64_t tag);
};

} // namespace PIMProf

#endif // __DATAREUSE_H__