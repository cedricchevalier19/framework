﻿// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2021 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* BasicReaderWriter.cc                                        (C) 2000-2021 */
/*                                                                           */
/* Lecture/Ecriture simple.                                                  */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "arcane/utils/TraceAccessor.h"
#include "arcane/utils/ScopedPtr.h"
#include "arcane/utils/StringBuilder.h"
#include "arcane/utils/OStringStream.h"
#include "arcane/utils/NotImplementedException.h"
#include "arcane/utils/NotSupportedException.h"
#include "arcane/utils/PlatformUtils.h"
#include "arcane/utils/IOException.h"
#include "arcane/utils/CheckedConvert.h"

#include "arcane/XmlNode.h"
#include "arcane/IXmlDocumentHolder.h"
#include "arcane/ISubDomain.h"
#include "arcane/IParallelMng.h"
#include "arcane/IIOMng.h"
#include "arcane/IDataReader.h"
#include "arcane/IDataReader2.h"
#include "arcane/IDataWriter.h"
#include "arcane/IDataFactoryMng.h"
#include "arcane/CheckpointService.h"
#include "arcane/Directory.h"
#include "arcane/ArcaneException.h"
#include "arcane/FactoryService.h"
#include "arcane/ISerializedData.h"
#include "arcane/IMesh.h"
#include "arcane/IRessourceMng.h"
#include "arcane/XmlNodeList.h"
#include "arcane/VariableCollection.h"
#include "arcane/IParallelReplication.h"
#include "arcane/IVariableUtilities.h"

#include "arcane/IDeflateService.h"
#include "arcane/ServiceFinder2.h"
#include "arcane/ServiceBuilder.h"

#include "arcane/VerifierService.h"
#include "arcane/IVariableMng.h"
#include "arcane/IItemFamily.h"

#include "arcane/utils/List.h"
#include "arcane/utils/ITraceMng.h"

#include "arcane/IParallelExchanger.h"
#include "arcane/ISerializeMessage.h"
#include "arcane/ISerializer.h"
#include "arcane/SerializeBuffer.h"
#include "arcane/VariableMetaData.h"
#include "arcane/CheckpointInfo.h"

#include "arcane/std/ParallelDataReader.h"
#include "arcane/std/ParallelDataWriter.h"

#include "arcane/std/TextReader.h"
#include "arcane/std/TextWriter.h"

#include <map>
#include <set>

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace Arcane
{

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

class VariableDataInfo
{
 public:
  VariableDataInfo(const String& full_name,const ISerializedData* sdata)
  : m_full_name(full_name)
  {
    m_nb_dimension = sdata->nbDimension();
    Int64ConstArrayView extents = sdata->extents();

    m_nb_base_element = sdata->nbBaseElement();

    m_nb_element = sdata->nbElement();
    m_is_multi_size = sdata->isMultiSize();
    m_dim2_size = 0;
    m_dim1_size = 0;
    if (m_nb_dimension==2 && !m_is_multi_size){
      m_dim1_size = extents[0];
      m_dim2_size = extents[1];
    }
    m_dimension_array_size = extents.size();
    m_base_data_type = sdata->baseDataType();
    m_memory_size = sdata->memorySize();
    m_file_offset = 0;
  }

  VariableDataInfo(const String& full_name,const XmlNode& element)
  : m_full_name(full_name)
  {
    m_nb_dimension = _readInteger(element,"nb-dimension");
    m_dim1_size = _readInt64(element,"dim1-size");
    m_dim2_size = _readInt64(element,"dim2-size");
    m_nb_element = _readInt64(element,"nb-element");
    m_nb_base_element = _readInt64(element,"nb-base-element");
    m_dimension_array_size = _readInteger(element,"dimension-array-size");
    m_is_multi_size = _readBool(element,"is-multi-size");
    m_base_data_type = (eDataType)_readInteger(element,"base-data-type");
    m_memory_size = _readInt64(element,"memory-size");
    m_file_offset = _readInt64(element,"file-offset");
  }

  const String& fullName() const { return m_full_name; }
  Integer nbDimension() const { return m_nb_dimension; }
  Int64 dim1Size() const { return m_dim1_size; }
  Int64 dim2Size() const { return m_dim2_size; }
  Int64 nbElement() const { return m_nb_element; }
  Int64 nbBaseElement() const { return m_nb_base_element; }
  Integer dimensionArraySize() const { return m_dimension_array_size; }
  bool isMultiSize() const { return m_is_multi_size; }
  eDataType baseDataType() const { return m_base_data_type; }
  Int64 memorySize() const { return m_memory_size; }

  void setFileOffset(Int64 v){ m_file_offset = v; }
  Int64 fileOffset() const { return m_file_offset; }

  void write(XmlNode element)
  {
    _addAttribute(element,"nb-dimension",m_nb_dimension);
    _addAttribute(element,"dim1-size",m_dim1_size);
    _addAttribute(element,"dim2-size",m_dim2_size);
    _addAttribute(element,"nb-element",m_nb_element);
    _addAttribute(element,"nb-base-element",m_nb_base_element);
    _addAttribute(element,"dimension-array-size",m_dimension_array_size);
    _addAttribute(element,"is-multi-size",(m_is_multi_size) ? 1 : 0);
    _addAttribute(element,"base-data-type",(Integer)m_base_data_type);
    _addAttribute(element,"memory-size",m_memory_size);
    _addAttribute(element,"file-offset",m_file_offset);
  }

 private:

  void _addAttribute(XmlNode& node,const String& attr_name,Int64 value)
  {
    node.setAttrValue(attr_name,String::fromNumber(value));
  }

  Integer _readInteger(const XmlNode& node,const String& attr_name)
  {
    return node.attr(attr_name,true).valueAsInteger(true);
  }

  Int64 _readInt64(const XmlNode& node,const String& attr_name)
  {
    return node.attr(attr_name,true).valueAsInt64(true);
  }

  bool _readBool(const XmlNode& node,const String& attr_name)
  {
    return node.attr(attr_name,true).valueAsBoolean(true);
  }

 private:

  String m_full_name;
  Integer m_nb_dimension;
  Int64 m_dim1_size;
  Int64 m_dim2_size;
  Int64 m_nb_element;
  Int64 m_nb_base_element;
  Integer m_dimension_array_size;
  bool m_is_multi_size;
  eDataType m_base_data_type;
  Int64 m_memory_size;
  Int64 m_file_offset;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

class BasicVariableMetaData
{
 public:
  BasicVariableMetaData(VariableMetaData* varmd)
  {
    m_full_name = varmd->fullName();
    m_item_group_name = varmd->itemGroupName();
    m_mesh_name = varmd->meshName();
    m_item_family_name = varmd->itemFamilyName();
  }
 public:
  const String& fullName() const { return m_full_name; }
  const String& itemGroupName() const { return m_item_group_name; }
  const String& meshName() const { return m_mesh_name; }
  const String& itemFamilyName() const { return m_item_family_name; }
  bool isItemVariable() const { return !m_item_family_name.null(); }
 private:
  String m_full_name;
  String m_item_group_name;
  String m_mesh_name;
  String m_item_family_name;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

namespace
{
String
_getBasicVariableFile(const String& path,Int32 rank)
{
  StringBuilder filename = path;
  filename += "/var___MAIN___";
  filename += rank;
  filename += ".txt";
  return filename;
}

String
_getBasicGroupFile(const String& path,const String& name,Int32 rank)
{
  StringBuilder filename = path;
  filename += "/group_";
  filename += name;
  filename += "_";
  filename += rank;
  filename += ".txt";
  return filename;
}

Ref<IDeflateService>
_createDeflater(IApplication* app,const String& name)
{
  ServiceBuilder<IDeflateService> sf(app);
  Ref<IDeflateService> bc = sf.createReference(app,name);
  return bc;
}

void
_fillUniqueIds(const ItemGroup& group,Array<Int64>& uids)
{
  Integer nb_item = group.size();
  uids.clear();
  uids.reserve(nb_item);
  ENUMERATE_ITEM(iitem,group){
    Int64 uid = iitem->uniqueId();
    uids.add(uid);
  }
}
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \internal
 * \brief Lecteur générique.
 */
class IGenericReader
{
 public:
  virtual ~IGenericReader(){}
 public:
  virtual void initialize(const String& path,Int32 rank) =0;
  virtual void readData(const String& var_full_name,IData* data) =0;
  virtual void readItemGroup(const String& group_name,Int64Array& written_unique_ids,
                             Int64Array& wanted_unique_ids) =0;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

class BasicGenericReader
: public TraceAccessor
, public IGenericReader
{
 public:
  BasicGenericReader(IApplication* app,bool is_binary)
  : TraceAccessor(app->traceMng()),
    m_application(app),
    m_is_binary(is_binary),
    m_rank(A_NULL_RANK),
    m_version(-1)
  {
  }
  ~BasicGenericReader() override
  {
    for (auto x : m_variables_data_info )
      delete x.second;
  }
 public:
  void initialize(const String& path,Int32 rank) override;
  void readData(const String& var_full_name,IData* data) override;
  void readItemGroup(const String& group_name,Int64Array& written_unique_ids,
                     Int64Array& wanted_unique_ids) override;
 private:
  IApplication* m_application;
  ScopedPtrT<KeyValueTextReader> m_text_reader;
  bool m_is_binary;
  String m_path;
  Int32 m_rank;
  Int32 m_version;
  typedef std::map<String,VariableDataInfo*> VariableDataInfoMap;
  VariableDataInfoMap m_variables_data_info;
 private:
  VariableDataInfo* _getVarInfo(const String& full_name);
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicGenericReader::
initialize(const String& path,Int32 rank)
{
  m_path = path;
  m_rank = rank;

  StringBuilder filename = m_path;
  filename += "/own_metadata_";
  filename += rank;
  filename += ".txt";

  info(4) << "Reading own metadata rank=" << rank << " file=" << filename;

  IApplication* app = m_application;
  ScopedPtrT<IXmlDocumentHolder> xdoc(app->ioMng()->parseXmlFile(filename));
  XmlNode root = xdoc->documentNode().documentElement();
  XmlNodeList variables_elem = root.children("variable-data");
  String deflater_name = root.attrValue("deflater-service");
  String version_id = root.attrValue("version",false);
  if (version_id.null() || version_id=="1")
    // Version 1:
    // - taille des dimensions sur 32 bits
    m_version = 1;
  else if (version_id=="2")
    // Version 2:
    // - taille des dimensions sur 64 bits
    m_version = 2;
  else
    ARCANE_FATAL("Unsupported version '{0}' (max=2)",version_id);

  Ref<IDeflateService> deflater;
  if (!deflater_name.null()){
    deflater = _createDeflater(m_application,deflater_name);
  }
  for( Integer i=0, is=variables_elem.size(); i<is; ++i ){
    XmlNode n = variables_elem[i];
    String var_full_name = n.attrValue("full-name");
    auto vdi = new VariableDataInfo(var_full_name,n);
    m_variables_data_info.insert(std::make_pair(var_full_name,vdi));
  }

  String main_filename = _getBasicVariableFile(m_path,rank);
  auto reader = new KeyValueTextReader(main_filename,m_is_binary,m_version);
  m_text_reader = reader;
  reader->setDeflater(deflater);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

VariableDataInfo* BasicGenericReader::
_getVarInfo(const String& full_name)
{
  VariableDataInfoMap::const_iterator ivar = m_variables_data_info.find(full_name);
  if (ivar==m_variables_data_info.end())
    ARCANE_THROW(ReaderWriterException,
                 "Can not find own metadata infos for data var={0} rank={1}",full_name,m_rank);
  VariableDataInfo* vdi = ivar->second;
  return vdi;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicGenericReader::
readData(const String& var_full_name,IData* data)
{
  KeyValueTextReader* reader = m_text_reader.get();
  String vname = var_full_name;
  VariableDataInfo* vdi = _getVarInfo(vname);
  reader->setFileOffset(vdi->fileOffset());

  eDataType data_type = vdi->baseDataType();
  Int64 memory_size = vdi->memorySize();
  Integer dimension_array_size = vdi->dimensionArraySize();
  Int64 nb_element = vdi->nbElement();
  Integer nb_dimension = vdi->nbDimension();
  Int64 nb_base_element = vdi->nbBaseElement();
  bool is_multi_size = vdi->isMultiSize();
  Int64UniqueArray extents;
  if (m_version==1 || m_version==2){
    String key_name = "VariableDim:" + var_full_name;
    if (m_version==1){
      // Dans la version 1, les dimensions sont des 'Int32'
      IntegerUniqueArray dims;
      if (dimension_array_size>0){
        dims.resize(dimension_array_size);
        extents.resize(dimension_array_size);
        reader->read(key_name,dims);
      }
      for( Integer i=0; i<dimension_array_size; ++i )
        extents[i] = dims[i];
    }
    else{
      if (dimension_array_size>0){
        extents.resize(dimension_array_size);
        reader->read(key_name,extents);
      }
    }
  }
  IDataFactoryMng* df = m_application->dataFactoryMng();
  Ref<ISerializedData> sd(df->createSerializedDataRef(data_type,memory_size,nb_dimension,nb_element,
                                                      nb_base_element,is_multi_size,extents));

  Int64 storage_size = sd->memorySize();
  info(4) << " READ DATA storage_size=" << storage_size << " DATA=" << data;

  data->allocateBufferForSerializedData(sd.get());

  void* ptr = sd->bytes().data();

  bool print_values = false;
  String key_name = var_full_name;
  if (storage_size!=0){
    eDataType base_data_type = sd->baseDataType();
    Real* real_ptr = (Real*)ptr;
    if (base_data_type==DT_Real){
      reader->read(key_name,Span<Real>(real_ptr,nb_base_element));
      if (print_values){
        if (nb_base_element>1){
          info() << "VAR=" << var_full_name << " offset=" << vdi->fileOffset()
                 << " V[0]=" << real_ptr[0] << " V[1]=" << real_ptr[1];
          const char* buf = (const char*)ptr;
          for( Integer i=0; i<16; ++i ){
            info() << "READ I=" << i << " V=" << (int)buf[i];
          }
        }
      }
    }
    else if (base_data_type==DT_Real2){
      reader->read(key_name,Span<Real>(real_ptr,nb_base_element*2));
    }
    else if (base_data_type==DT_Real3){
      reader->read(key_name,Span<Real>(real_ptr,nb_base_element*3));
    }
    else if (base_data_type==DT_Real2x2){
      reader->read(key_name,Span<Real>(real_ptr,nb_base_element*4));
    }
    else if (base_data_type==DT_Real3x3){
      reader->read(key_name,Span<Real>(real_ptr,nb_base_element*9));
    }
    else if (base_data_type==DT_Int16){
      reader->read(key_name,Span<Int16>((Int16*)ptr,nb_base_element));
    }
    else if (base_data_type==DT_Int32){
      reader->read(key_name,Span<Int32>((Int32*)ptr,nb_base_element));
    }
    else if (base_data_type==DT_Int64){
      reader->read(key_name,Span<Int64>((Int64*)ptr,nb_base_element));
    }
    else if (base_data_type==DT_Byte){
      reader->read(key_name,Span<Byte>((Byte*)ptr,storage_size));
    }
    else
      ARCANE_THROW(NotSupportedException,"Bad datatype {0}",base_data_type);
  }

  data->assignSerializedData(sd.get());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicGenericReader::
readItemGroup(const String& group_full_name,Int64Array& written_unique_ids,
              Int64Array& wanted_unique_ids)
{
  info(5) << "READ GROUP " << group_full_name;
  String filename = _getBasicGroupFile(m_path,group_full_name,m_rank);
  TextReader reader(filename,m_is_binary);

  {
    Integer nb_unique_id = 0;
    reader.readIntegers(IntegerArrayView(1,&nb_unique_id));
    info(5) << "NB_WRITTEN_UNIQUE_ID = " << nb_unique_id;
    written_unique_ids.resize(nb_unique_id);
    reader.read(written_unique_ids);
  }

  {
    Integer nb_unique_id = 0;
    reader.readIntegers(IntegerArrayView(1,&nb_unique_id));
    info(5) << "NB_WANTED_UNIQUE_ID = " << nb_unique_id;
    wanted_unique_ids.resize(nb_unique_id);
    reader.read(wanted_unique_ids);
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \internal
 * \brief Écrivain générique.
 */
class IGenericWriter
{
 public:
  virtual ~IGenericWriter(){}
 public:
  virtual void initialize(const String& path,Int32 rank) =0;
  virtual void writeData(const String& var_full_name,const ISerializedData* sdata) =0;
  virtual void writeItemGroup(const String& group_full_name,
                              Int64ConstArrayView written_unique_ids,
                              Int64ConstArrayView wanted_unique_ids) =0;
  virtual void endWrite() =0;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

class BasicGenericWriter
: public TraceAccessor
, public IGenericWriter
{
 public:
  BasicGenericWriter(IApplication* app,bool is_binary,Int32 version)
  : TraceAccessor(app->traceMng()),
    m_application(app),
    m_is_binary(is_binary),
    m_version(version),
    m_rank(A_NULL_RANK)
  {
  }
  ~BasicGenericWriter()
  {
    for( auto x : m_variables_data_info ){
      delete x.second;
    }
  }
 public:
  void initialize(const String& path,Int32 rank) override
  {
    m_path = path;
    m_rank = rank;
    String deflater_name = platform::getEnvironmentVariable("ARCANE_DEFLATER");
    if (!deflater_name.null()){
      m_write_deflater_name = deflater_name;
      auto bc = _createDeflater(m_application,m_write_deflater_name);
      info() << "Use deflater name=" << deflater_name;
      m_text_writer->setDeflater(bc);
    }
    String filename = _getBasicVariableFile(path,rank);
    m_text_writer = new KeyValueTextWriter(filename,m_is_binary,m_version);
  }
  void writeData(const String& var_full_name,const ISerializedData* sdata) override;
  void writeItemGroup(const String& group_full_name,Int64ConstArrayView written_unique_ids,
                      Int64ConstArrayView wanted_unique_ids) override;
  void endWrite() override;
 private:
  IApplication* m_application;
  bool m_is_binary;
  Int32 m_version;
  String m_write_deflater_name;
  String m_path;
  Int32 m_rank;
  ScopedPtrT<KeyValueTextWriter> m_text_writer;
  typedef std::map<String,VariableDataInfo*> VariableDataInfoMap;
  VariableDataInfoMap m_variables_data_info;
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicGenericWriter::
writeData(const String& var_full_name,const ISerializedData* sdata)
{
  //TODO: Verifier que initialize() a bien été appelé.
  auto var_data_info = new VariableDataInfo(var_full_name,sdata);
  KeyValueTextWriter* writer = m_text_writer.get();
  var_data_info->setFileOffset(writer->fileOffset());
  m_variables_data_info.insert(std::make_pair(var_full_name,var_data_info));
  info(4) << " SDATA name=" << var_full_name << " nb_element=" << sdata->nbElement()
          << " dim=" << sdata->nbDimension() << " datatype=" << sdata->baseDataType()
          << " nb_basic_element=" << sdata->nbBaseElement()
          << " is_multi=" << sdata->isMultiSize()
          << " dimensions_size=" << sdata->extents().size()
          << " memory_size=" << sdata->memorySize()
          << " bytes_size=" << sdata->bytes().size();

  const void* ptr = sdata->bytes().data();

  // Si la variable est de type tableau à deux dimensions, sauve les
  // tailles de la deuxième dimension par élément.
  Int64ConstArrayView extents = sdata->extents();
  if (m_version==1 || m_version==2){
    Integer dimension_array_size = extents.size();
    if (dimension_array_size!=0){
      String key_name = "VariableDim:" + var_full_name;
      String comment = String::format("Writing Dim1Size for '{0}'",var_full_name);
      if (m_version==1){
        UniqueArray<Integer> dims(dimension_array_size);
        for( Integer i=0; i<dimension_array_size; ++i )
          dims[i] = CheckedConvert::toInteger(extents[i]);
        writer->write(key_name,comment,Int64ConstArrayView(),dims);
      }
      else
        writer->write(key_name,comment,Int64ConstArrayView(),extents);
    }
  }

  // Maintenant, sauve les valeurs si necessaire
  Int64 nb_base_element = sdata->nbBaseElement();
  if (nb_base_element!=0 && ptr){
    String key_name = var_full_name;
    String comment = String::format("Write Values for '{0}'",var_full_name);
    eDataType base_data_type = sdata->baseDataType();
    if (base_data_type==DT_Real){
      writer->write(key_name,comment,extents,Span<const Real>((const Real*)ptr,nb_base_element));
    }
    else if (base_data_type==DT_Real2){
      writer->write(key_name,comment,extents,Span<const Real>((const Real*)ptr,nb_base_element*2));
    }
    else if (base_data_type==DT_Real3){
      writer->write(key_name,comment,extents,Span<const Real>((const Real*)ptr,nb_base_element*3));
    }
    else if (base_data_type==DT_Real2x2){
      writer->write(key_name,comment,extents,Span<const Real>((const Real*)ptr,nb_base_element*4));
    }
    else if (base_data_type==DT_Real3x3){
      writer->write(key_name,comment,extents,Span<const Real>((const Real*)ptr,nb_base_element*9));
    }
    else if (base_data_type==DT_Int16){
      writer->write(key_name,comment,extents,Span<const Int16>((const Int16*)ptr,nb_base_element));
    }
    else if (base_data_type==DT_Int32){
      writer->write(key_name,comment,extents,Span<const Int32>((const Int32*)ptr,nb_base_element));
    }
    else if (base_data_type==DT_Int64){
      writer->write(key_name,comment,extents,Span<const Int64>((const Int64*)ptr,nb_base_element));
    }
    else if (base_data_type==DT_Byte){
      writer->write(key_name,comment,extents,Span<const Byte>((const Byte*)ptr,nb_base_element));
    }
    else
      ARCANE_THROW(NotSupportedException,"Bad datatype {0}",base_data_type);
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicGenericWriter::
writeItemGroup(const String& group_full_name,Int64ConstArrayView written_unique_ids,
               Int64ConstArrayView wanted_unique_ids)
{
  String filename = _getBasicGroupFile(m_path,group_full_name,m_rank);
  TextWriter writer(filename,m_is_binary);

  // Sauve la liste des unique_ids écrits
  {
    Integer nb_unique_id = written_unique_ids.size();
    writer.write("Nb written uniqueIds",IntegerConstArrayView(1,&nb_unique_id));
    writer.write("Written UniqueIds",written_unique_ids);
  }

  // Sauve la liste des unique_ids souhaités par ce sous-domaine
  {
    Integer nb_unique_id = wanted_unique_ids.size();
    writer.write("Nb wanted uniqueIds",IntegerConstArrayView(1,&nb_unique_id));
    writer.write("Writing Wanted UniqueIds",wanted_unique_ids);
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicGenericWriter::
endWrite()
{
  Int32 rank = m_rank;

  StringBuilder filename = m_path;
  filename += "/own_metadata_";
  filename += rank;
  filename += ".txt";

  IApplication* app = m_application;
  ScopedPtrT<IXmlDocumentHolder> xdoc(app->ressourceMng()->createXmlDocument());
  XmlNode doc = xdoc->documentNode();
  XmlElement root(doc,"variables-data");
  if (!m_write_deflater_name.null())
    root.setAttrValue("deflater-service",m_write_deflater_name);
  root.setAttrValue("version",String::fromNumber(m_version));
  for( auto i : m_variables_data_info ){
    VariableDataInfo* vdi = i.second;
    XmlNode e = root.createAndAppendElement("variable-data");
    e.setAttrValue("full-name",vdi->fullName());
    vdi->write(e);
  }
  String fn = filename.toString();
  ofstream ofile(fn.localstr());
  app->ioMng()->writeXmlFile(xdoc.get(),filename);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

class BasicReaderWriterCommon
: public TraceAccessor
{
 public:
  enum eOpenMode
  {
    OpenModeRead,
    OpenModeTruncate,
    OpenModeAppend
  };
 public:
  BasicReaderWriterCommon(IApplication* app,IParallelMng* pm,
                          const String& path,eOpenMode open_mode);
  ~BasicReaderWriterCommon();
 protected:
  IApplication* m_application;
  IParallelMng* m_parallel_mng;
  eOpenMode m_open_mode;
  String m_path;
  Integer m_verbose_level;
 protected:
  String _getMetaDataFileName(Int32 rank);
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

BasicReaderWriterCommon::
BasicReaderWriterCommon(IApplication* app,IParallelMng* pm,
                        const String& path,eOpenMode open_mode)
: TraceAccessor(pm->traceMng())
, m_application(app)
, m_parallel_mng(pm)
, m_open_mode(open_mode)
, m_path(path)
, m_verbose_level(0)
{}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

BasicReaderWriterCommon::
~BasicReaderWriterCommon()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

String BasicReaderWriterCommon::
_getMetaDataFileName(Int32 rank)
{
  StringBuilder filename = m_path;
  filename += "/metadata";
  filename += "-";
  filename += rank;
  filename += ".txt";
  return filename;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Lecture/Ecriture simple.
 */
class BasicWriter
: public BasicReaderWriterCommon
, public IDataWriter
{
 public:

  BasicWriter(IApplication* app,IParallelMng* pm,const String& path,
              eOpenMode open_mode,Integer version,bool want_parallel);
  ~BasicWriter() override;

 public:

  void initialize();

  void beginWrite(const VariableCollection& vars) override;
  void endWrite() override;

  void setMetaData(const String& meta_data) override;

  void write(IVariable* v,IData* data) override;

 private:

  bool m_want_parallel;
  bool m_is_binary;
  bool m_is_gather;

  std::map<ItemGroup,ParallelDataWriter*> m_parallel_data_writers;
  std::set<ItemGroup> m_written_groups;

  IGenericWriter* m_global_writer;

 private:

  void _directWriteVal(IVariable* v,IData* data);
  void _writeVal(TextWriter* writer,VariableDataInfo* data_info,
                 const ISerializedData* sdata);

  ParallelDataWriter* _getWriter(IVariable* var);
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

BasicWriter::
BasicWriter(IApplication* app,IParallelMng* pm,const String& path,
            eOpenMode open_mode,Integer version,bool want_parallel)
: BasicReaderWriterCommon(app,pm,path,open_mode)
, m_want_parallel(want_parallel)
, m_is_binary(true)
, m_is_gather(false)
, m_global_writer(nullptr)
{
  m_global_writer = new BasicGenericWriter(app,m_is_binary,version);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

BasicWriter::
~BasicWriter()
{
  delete m_global_writer;
  for( auto i : m_parallel_data_writers )
    delete i.second;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicWriter::
initialize()
{
  if (m_verbose_level>0)
    info() << "** OPEN MODE = " << m_open_mode;
  if (m_open_mode==OpenModeTruncate && m_parallel_mng->isMasterIO())
    platform::recursiveCreateDirectory(m_path);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

ParallelDataWriter* BasicWriter::
_getWriter(IVariable* var)
{
  ItemGroup group = var->itemGroup();
  auto i = m_parallel_data_writers.find(group);
  if (i!=m_parallel_data_writers.end())
    return i->second;
  ParallelDataWriter* writer = new ParallelDataWriter(m_parallel_mng);
  writer->setGatherAll(m_is_gather);
  {
    Int64UniqueArray items_uid;
    ItemGroup own_group = var->itemGroup().own();
    _fillUniqueIds(own_group,items_uid);
    Int32ConstArrayView local_ids = own_group.internal()->itemsLocalId();
    writer->sort(local_ids,items_uid);
  }
  m_parallel_data_writers.insert(std::make_pair(group,writer));
  return writer;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicWriter::
_directWriteVal(IVariable* var,IData* data)
{
  info(4) << "DIRECT WRITE VAL v=" << var->fullName();

  IData* write_data = data;
  Int64ConstArrayView written_unique_ids;
  Int64UniqueArray wanted_unique_ids;
  Int64UniqueArray sequential_written_unique_ids;
  Ref<IData> allocated_write_data;
  if (var->itemKind()!=IK_Unknown){
    ItemGroup group = var->itemGroup();
    if (m_want_parallel){
      ParallelDataWriter* writer = _getWriter(var);
      written_unique_ids = writer->sortedUniqueIds();
      allocated_write_data = writer->getSortedValues(data);
      write_data = allocated_write_data.get();
    }
    else{
      //TODO il faut trier les uniqueId
      _fillUniqueIds(group,sequential_written_unique_ids);
      written_unique_ids = sequential_written_unique_ids.view();
    }
    _fillUniqueIds(group,wanted_unique_ids);
    if (m_written_groups.find(group)==m_written_groups.end()){
      info(5) << "WRITE GROUP " << group.name();
      IItemFamily* item_family = group.itemFamily();
      String gname = group.name();
      String group_full_name = item_family->fullName() + "_" + gname;
      m_global_writer->writeItemGroup(group_full_name,written_unique_ids,wanted_unique_ids);
      m_written_groups.insert(group);
    }
  }
  Ref<ISerializedData> sdata(write_data->createSerializedDataRef(false));
  m_global_writer->writeData(var->fullName(),sdata.get());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicWriter::
write(IVariable* var,IData* data)
{
  if (var->isPartial()){
    info() << "** WARNING: partial variable not implemented in BasicReaderWriter";
    return;
  }
  _directWriteVal(var,data);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicWriter::
setMetaData(const String& meta_data)
{
  Int32 my_rank = m_parallel_mng->commRank();
  String filename = _getMetaDataFileName(my_rank);
  ofstream ofile(filename.localstr());
  meta_data.writeBytes(ofile);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicWriter::
beginWrite(const VariableCollection& vars)
{
  ARCANE_UNUSED(vars);
  Int32 my_rank = m_parallel_mng->commRank();
  m_global_writer->initialize(m_path,my_rank);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicWriter::
endWrite()
{
  IParallelMng* pm = m_parallel_mng;
  if (pm->isMasterIO()){
    StringBuilder filename = m_path;
    filename += "/infos.txt";
    String fn = filename.toString();
    ofstream ofile(fn.localstr());
    ofile << pm->commSize() << '\n';
  }
  m_global_writer->endWrite();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Lecteur simple.
 */
class BasicReader
: public BasicReaderWriterCommon
, public IDataReader
, public IDataReader2
{
 public:
  /*!
    \brief  Interface pour retrouver le groupe associée à une variable à partir
    de ces meta-données.
  */
  class IItemGroupFinder
  {
   public:
    virtual ~IItemGroupFinder(){}
    virtual ItemGroup getWantedGroup(VariableMetaData* vmd) =0;
  };
 public:

  BasicReader(IApplication* app,IParallelMng* pm,Int32 forced_rank_to_read,
              const String& path,bool want_parallel);
  ~BasicReader() override;

 public:

  void beginRead(const VariableCollection& vars) override;
  void endRead() override {}
  String metaData() override;
  void read(IVariable* v,IData* data) override;

  void fillMetaData(ByteArray& bytes) override;
  void beginRead(const DataReaderInfo& infos) override;
  void read(const VariableDataReadInfo& infos) override;

 public:

  void initialize();
  void setItemGroupFinder(IItemGroupFinder* group_finder)
  {
    m_item_group_finder = group_finder;
  }
 private:

  bool m_want_parallel;
  Integer m_nb_written_part;
  bool m_is_binary;

  Int32 m_first_rank_to_read;
  Int32 m_nb_rank_to_read;
  Int32 m_forced_rank_to_read;

  std::map<String,ParallelDataReader*> m_parallel_data_readers;
  UniqueArray<IGenericReader*> m_global_readers;
  IItemGroupFinder* m_item_group_finder;

 private:

  void _directReadVal(VariableMetaData* varmd,IData* data);

  ParallelDataReader* _getReader(VariableMetaData* varmd);
  void _setRanksToRead();
  IGenericReader* _readOwnMetaDataAndCreateReader(Int32 rank);
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

BasicReader::
BasicReader(IApplication* app,IParallelMng* pm,Int32 forced_rank_to_read,
            const String& path,bool want_parallel)
: BasicReaderWriterCommon(app,pm,path,BasicReaderWriterCommon::OpenModeRead)
, m_want_parallel(want_parallel)
, m_nb_written_part(0)
, m_is_binary(true)
, m_first_rank_to_read(0)
, m_nb_rank_to_read(0)
, m_forced_rank_to_read(forced_rank_to_read)
, m_item_group_finder(nullptr)
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicReader::
initialize()
{
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

BasicReader::
~BasicReader()
{
  for( auto i : m_parallel_data_readers )
    delete i.second;
  for( auto r : m_global_readers )
    delete r;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicReader::
_directReadVal(VariableMetaData* varmd,IData* data)
{
  info(4) << "DIRECT READ VAL v=" << varmd->fullName();

  bool is_item_variable = !varmd->itemFamilyName().null();
  Int32 nb_rank_to_read = m_nb_rank_to_read;
  // S'il s'agit d'une variable qui n'est pas sur le maillage,
  // il ne faut lire qu'un seul rang car il n'est pas
  // certain que cette variable soit definie partout
  if (!is_item_variable)
    if (nb_rank_to_read>1)
      nb_rank_to_read = 1;
        
  UniqueArray<Ref<IData>> allocated_data;
  UniqueArray<IData*> written_data(nb_rank_to_read);

  for( Integer i=0; i<nb_rank_to_read; ++i ){
    written_data[i] = data;
    if ((nb_rank_to_read>1 || m_want_parallel) && is_item_variable){
      Ref<IData> new_data = data->cloneEmptyRef();
      written_data[i] = new_data.get();
      allocated_data.add(new_data);
    }
    String vname = varmd->fullName();
    info(4) << " TRY TO READ var_full_name=" << vname;
    m_global_readers[i]->readData(vname,written_data[i]);
  }
  
  if (is_item_variable){
    ParallelDataReader* parallel_data_reader = _getReader(varmd);

    Int64UniqueArray full_written_unique_ids;
    IData* full_written_data = nullptr;

    if (nb_rank_to_read==0){
      // Rien à lire
      // Il faut tout de même passer dans le reader parallèle
      // pour assurer les opérations collectives
      full_written_data = nullptr;
    }
    else if (nb_rank_to_read==1){
      //full_written_unique_ids = written_unique_ids[0];
      full_written_data = written_data[0];
    }
    else{
      // Il faut créer une donnée qui contient l'union des written_data
      Ref<IData> allocated_written_data = data->cloneEmptyRef();
      allocated_data.add(allocated_written_data);
      full_written_data = allocated_written_data.get();
      SerializeBuffer sbuf;
      sbuf.setMode(ISerializer::ModeReserve);
      for( Int32 i=0; i<nb_rank_to_read; ++i )
        written_data[i]->serialize(&sbuf,nullptr);
      sbuf.allocateBuffer();
      sbuf.setMode(ISerializer::ModePut);
      for( Int32 i=0; i<nb_rank_to_read; ++i )
        written_data[i]->serialize(&sbuf,nullptr);
      sbuf.setMode(ISerializer::ModeGet);
      sbuf.setReadMode(ISerializer::ReadAdd);
      for( Int32 i=0; i<nb_rank_to_read; ++i )
        full_written_data->serialize(&sbuf,nullptr);

    }
    if (data!=full_written_data){
      info(5) << "PARALLEL READ";
      parallel_data_reader->getSortedValues(full_written_data,data);
    }
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

ParallelDataReader* BasicReader::
_getReader(VariableMetaData* varmd)
{
  Int32 nb_to_read = m_nb_rank_to_read;

  // Pour la lecture, lors d'une reprise, le groupe (var->itemGroup())
  // associé à la variable ainsi que la famille (var->itemFamily())
  // n'existe pas encore. Il ne faut donc pas l'utiliser
  const String& var_group_name = varmd->itemGroupName();
  String group_full_name = varmd->meshName()+ "_" + varmd->itemFamilyName() + "_" + var_group_name;
  auto ix = m_parallel_data_readers.find(group_full_name);
  if (ix!=m_parallel_data_readers.end())
    return ix->second;

  IParallelMng* pm = m_parallel_mng;
  auto reader = new ParallelDataReader(pm);
  {
    UniqueArray<SharedArray<Int64> > written_unique_ids(nb_to_read);
    Int64Array& wanted_unique_ids = reader->wantedUniqueIds();
    for( Integer i=0; i<nb_to_read; ++i ){
      m_global_readers[i]->readItemGroup(group_full_name,written_unique_ids[i],wanted_unique_ids);
    }
    // Cela ne doit être actif que pour les comparaisons (pas en reprise car les groupes
    // ne sont pas valides)
    if (m_item_group_finder){
      ItemGroup ig = m_item_group_finder->getWantedGroup(varmd);
      _fillUniqueIds(ig,wanted_unique_ids);
    }
    for( Integer i=0; i<nb_to_read; ++i ){
      Integer nb_uid = written_unique_ids[i].size();
      if (nb_uid>=1)
        info(5) << "PART I=" << i
               << " nb_uid=" << nb_uid
               << " min_uid=" << written_unique_ids[i][0]
               << " max_uid=" << written_unique_ids[i][nb_uid-1];
    }
    Int64Array& full_written_unique_ids = reader->writtenUniqueIds();
    for( Integer i=0; i<nb_to_read; ++i )
      for( Integer z=0, zs=written_unique_ids[i].size(); z<zs; ++z )
        full_written_unique_ids.add(written_unique_ids[i][z]);
    info(5) << "FULL UID SIZE=" << full_written_unique_ids.size();
    if (m_want_parallel)
      reader->sort();
  }
  m_parallel_data_readers.insert(std::make_pair(group_full_name,reader));
  return reader;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicReader::
read(IVariable* var,IData* data)
{
  info(4) << "MASTER READ var=" << var->fullName() << " data=" << data;
  if (var->isPartial()){
    info() << "** WARNING: partial variable not implemented in BasicReaderWriter";
    return;
  }
  ScopedPtrT<VariableMetaData> vmd(var->createMetaData());
  _directReadVal(vmd.get(),data);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicReader::
read(const VariableDataReadInfo& infos)
{
  VariableMetaData* vmd = infos.variableMetaData();
  IData* data = infos.data();
  info(4) << "MASTER2 READ var=" << vmd->fullName() << " data=" << data;
  if (vmd->isPartial()){
    info() << "** WARNING: partial variable not implemented in BasicReaderWriter";
    return;
  }
  _directReadVal(vmd,data);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

String BasicReader::
metaData()
{
  ByteUniqueArray bytes;
  fillMetaData(bytes);
  String s(bytes);
  info(5) << " S=" << s << '\n';
  return s;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicReader::
fillMetaData(ByteArray& bytes)
{
  Int32 rank = m_parallel_mng->commRank();
  if (m_forced_rank_to_read>=0)
    rank = m_forced_rank_to_read;
  String filename = _getMetaDataFileName(rank);
  info(4) << "Reading metadata file=" << filename;
  ifstream ifile(filename.localstr());
  Int64 file_length = platform::getFileLength(filename);
  bytes.resize(file_length);
  ifile.read((char*)bytes.data(),file_length);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicReader::
_setRanksToRead()
{
  IParallelMng* pm = m_parallel_mng;
  Int32 my_rank = pm->commRank();
  Int32 nb_rank = pm->commSize();
  Int32 first_to_read = my_rank;
  Int32 nb_to_read = 1;

  if (m_forced_rank_to_read>=0){
    m_first_rank_to_read = m_forced_rank_to_read;
    m_nb_rank_to_read = 1;
    return;
  }
  if (nb_rank==1){
    nb_to_read = m_nb_written_part;
  }
  else if (nb_rank<m_nb_written_part){
    // Il y a plus de fichiers que de processeurs.
    // Un des processeurs doit donc lire au moins deux fichiers.
    // Pour ceux dont c'est le cas, il faut que ces fichiers
    // soient consécutifs pour que l'intervalle des uniqueId()
    // des entités du processeur soient consécutifs
    Int32 nb_part_per_rank = m_nb_written_part / nb_rank;
    Int32 remaining_nb_part = m_nb_written_part - (nb_part_per_rank*nb_rank);
    first_to_read = nb_part_per_rank * my_rank;
    nb_to_read = nb_part_per_rank;
    info(4) << "NB_PART_PER_RANK = " << nb_part_per_rank
            << " REMAINING=" << remaining_nb_part;
    if (my_rank>=remaining_nb_part){
      first_to_read += remaining_nb_part;
    }
    else{
      first_to_read += my_rank;
      ++nb_to_read;
    }
  }
  else{
    if (my_rank>=m_nb_written_part){
      // Aucune partie à lire
      nb_to_read = 0;
    }
  }

  m_first_rank_to_read = first_to_read;
  m_nb_rank_to_read = nb_to_read;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

IGenericReader* BasicReader::
_readOwnMetaDataAndCreateReader(Int32 rank)
{
  auto r = new BasicGenericReader(m_application,m_is_binary);
  r->initialize(m_path,rank);
  return r;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicReader::
beginRead(const VariableCollection& vars)
{
  ARCANE_UNUSED(vars);
  beginRead(DataReaderInfo());
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void BasicReader::
beginRead(const DataReaderInfo& infos)
{
  ARCANE_UNUSED(infos);
  info() << "** ** BEGIN READ";
  IParallelMng* pm = m_parallel_mng;
  if (pm->isMasterIO()){
    String filename = String::concat(m_path,"/infos.txt");
    ifstream ifile(filename.localstr());
    Integer nb_part = 0;
    ifile >> nb_part;
    info() << "** NB PART=" << nb_part;
    m_nb_written_part = nb_part;
  }
  pm->broadcast(IntegerArrayView(1,&m_nb_written_part),pm->masterIORank());

  _setRanksToRead();
  info(4) << "RanksToRead: FIRST TO READ =" << m_first_rank_to_read
          << " nb=" << m_nb_rank_to_read;
  m_global_readers.resize(m_nb_rank_to_read);
  for( Integer i=0; i<m_nb_rank_to_read; ++i )
    m_global_readers[i] = _readOwnMetaDataAndCreateReader(i+m_first_rank_to_read);
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Protection/reprise basique (version 1).
 */
class ArcaneBasicCheckpointService
: public CheckpointService
{
 public:

  explicit ArcaneBasicCheckpointService(const ServiceBuildInfo& sbi)
  : CheckpointService(sbi), m_write_index(0), m_writer(nullptr), m_reader(nullptr)
  { }
  IDataWriter* dataWriter() override { return m_writer; }
  IDataReader* dataReader() override { return m_reader; }

  void notifyBeginWrite() override;
  void notifyEndWrite() override;
  void notifyBeginRead() override;
  void notifyEndRead() override;
  void close() override {}
  String readerServiceName() const override { return "ArcaneBasicCheckpointReader"; }

 private:

  Integer m_write_index;
  BasicWriter* m_writer;
  BasicReader* m_reader;

 private:

  String _defaultFileName()
  {
    info() << "USE DEFAULT FILE NAME index=" << currentIndex();
    IParallelReplication* pr = subDomain()->parallelMng()->replication();

    String buf = "arcanedump";
    if (pr->hasReplication()){
      buf = buf + "_r";
      buf = buf + pr->replicationRank();
    }
    info() << "FILE_NAME is " << buf;
    return buf;
  }
  Directory _defaultDirectory()
  {
    return Directory(baseDirectoryName());
  }
  void _parseMetaData(const String& meta_data);
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Protection/reprise basique (version 2).
 *
 * Idem ArcaneBasicCheckpointService sauf qu'on utilise le
 * service ArcaneBasic2CheckpointReader pour la relecture.
 */
class ArcaneBasic2CheckpointService
: public ArcaneBasicCheckpointService
{
 public:

  explicit ArcaneBasic2CheckpointService(const ServiceBuildInfo& sbi)
  : ArcaneBasicCheckpointService(sbi)
  { }
  String readerServiceName() const override { return "ArcaneBasic2CheckpointReader"; }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void ArcaneBasicCheckpointService::
_parseMetaData(const String& meta_data)
{
  IIOMng* io_mng = subDomain()->ioMng();
  ScopedPtrT<IXmlDocumentHolder> xml_doc(io_mng->parseXmlBuffer(meta_data.utf8(),"MetaData"));
  XmlNode root = xml_doc->documentNode().documentElement();
  Integer version = root.attr("version").valueAsInteger();
  if (version!=1){
    ARCANE_THROW(ReaderWriterException,"Bad version '{0}' (expected 1)",version);
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void ArcaneBasicCheckpointService::
notifyBeginRead()
{
  String meta_data = readerMetaData();
  _parseMetaData(meta_data);

  info() << " GET META DATA READER " << readerMetaData()
         << " filename=" << fileName();

  String filename = fileName();
  if (filename.null()){
    Directory dump_dir(_defaultDirectory());
    filename = dump_dir.file(_defaultFileName());
    setFileName(filename);
  }
  filename = filename + "_n" + currentIndex();
  info() << " READ CHECKPOINT FILENAME = " << filename;
  IParallelMng* pm = subDomain()->parallelMng();
  IApplication* app = subDomain()->application();
  bool want_parallel = false;
  m_reader = new BasicReader(app,pm,A_NULL_RANK,filename,want_parallel);
  m_reader->initialize();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void ArcaneBasicCheckpointService::
notifyEndRead()
{
  delete m_reader;
  m_reader = nullptr;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void ArcaneBasicCheckpointService::
notifyBeginWrite()
{
  auto open_mode = BasicReaderWriterCommon::OpenModeAppend;
  Integer write_index = checkpointTimes().size();
  --write_index;
  if (write_index==0)
    open_mode = BasicReaderWriterCommon::OpenModeTruncate;

  String filename = fileName();
  if (filename.null()){
    Directory dump_dir(_defaultDirectory());
    filename = dump_dir.file(_defaultFileName());
    setFileName(filename);
  }
  filename = filename + "_n" + write_index;

  platform::recursiveCreateDirectory(filename);

  IParallelMng* pm = subDomain()->parallelMng();
  IApplication* app = subDomain()->application();
  bool want_parallel = pm->isParallel();
  want_parallel = false;
  m_writer = new BasicWriter(app,pm,filename,open_mode,2,want_parallel);
  m_writer->initialize();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void ArcaneBasicCheckpointService::
notifyEndWrite()
{
  OStringStream ostr;
  ostr() << "<infos version='1'>\n";
  ostr() << "</infos>\n";
  setReaderMetaData(ostr.str());
  ++m_write_index;
  delete m_writer;
  m_writer = nullptr;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*!
 * \brief Relecture de protection (version 2).
 */
class ArcaneBasic2CheckpointReaderService
: public AbstractService
, public ICheckpointReader2
{
 public:

  explicit ArcaneBasic2CheckpointReaderService(const ServiceBuildInfo& sbi)
  : AbstractService(sbi), m_application(sbi.application()), m_reader(nullptr)
  { }
  IDataReader2* dataReader() override { return m_reader; }

  void notifyBeginRead(const CheckpointReadInfo& cri) override;
  void notifyEndRead() override;

 private:

  IApplication* m_application;
  BasicReader* m_reader;

 private:

  String _defaultFileName(const CheckpointInfo& ci)
  {
    Integer index = ci.checkpointIndex();
    bool has_replication = ci.nbReplication()>1;
    Int32 replication_rank = ci.replicationRank();
    info() << "USE DEFAULT FILE NAME index=" << index;

    String buf = "arcanedump";
    if (has_replication){
      buf = buf + "_r";
      buf = buf + replication_rank;
    }
    info() << "FILE_NAME is " << buf;
    return buf;
  }
  void _parseMetaData(const String& meta_data);
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void ArcaneBasic2CheckpointReaderService::
_parseMetaData(const String& meta_data)
{
  auto doc_ptr = IXmlDocumentHolder::loadFromBuffer(meta_data.bytes(),"MetaData",traceMng());
  ScopedPtrT<IXmlDocumentHolder> xml_doc(doc_ptr);
  XmlNode root = xml_doc->documentNode().documentElement();
  Integer version = root.attr("version").valueAsInteger();
  if (version!=1){
    ARCANE_THROW(ReaderWriterException,"Bad version '{0}' (expected 1)",version);
  }
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void ArcaneBasic2CheckpointReaderService::
notifyBeginRead(const CheckpointReadInfo& cri)
{
  const CheckpointInfo& ci = cri.checkpointInfo();
  String reader_meta_data = ci.readerMetaData();
  _parseMetaData(reader_meta_data);

  info() << "Basic2CheckpointReader GET META DATA READER " << reader_meta_data;

  Directory dump_dir(ci.directory());
  String filename = dump_dir.file(_defaultFileName(ci));
  filename = filename + "_n" + ci.checkpointIndex();;
  info() << " READ CHECKPOINT FILENAME = " << filename;
  IParallelMng* pm = cri.parallelMng();
  bool want_parallel = false;
  m_reader = new BasicReader(m_application,pm,ci.subDomainRank(),filename,want_parallel);
  m_reader->initialize();
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

void ArcaneBasic2CheckpointReaderService::
notifyEndRead()
{
  delete m_reader;
  m_reader = nullptr;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

ARCANE_REGISTER_SERVICE(ArcaneBasicCheckpointService,
                        ServiceProperty("ArcaneBasicCheckpointWriter",ST_SubDomain|ST_CaseOption),
                        ARCANE_SERVICE_INTERFACE(Arcane::ICheckpointWriter));

ARCANE_REGISTER_SERVICE(ArcaneBasicCheckpointService,
                        ServiceProperty("ArcaneBasicCheckpointReader",ST_SubDomain),
                        ARCANE_SERVICE_INTERFACE(Arcane::ICheckpointReader));

ARCANE_REGISTER_SERVICE(ArcaneBasic2CheckpointService,
                        ServiceProperty("ArcaneBasic2CheckpointWriter",ST_SubDomain|ST_CaseOption),
                        ARCANE_SERVICE_INTERFACE(Arcane::ICheckpointWriter));

ARCANE_REGISTER_SERVICE(ArcaneBasic2CheckpointReaderService,
                        ServiceProperty("ArcaneBasic2CheckpointReader",ST_Application),
                        ARCANE_SERVICE_INTERFACE(Arcane::ICheckpointReader2));

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

class ArcaneBasicVerifierService2
: public VerifierService
{
  class GroupFinder
  : public BasicReader::IItemGroupFinder
  {
   public:
    GroupFinder(IVariableMng* vm) : m_variable_mng(vm){}
    ItemGroup getWantedGroup(VariableMetaData* vmd) override
    {
      String full_name = vmd->fullName();
      IVariable* var = m_variable_mng->findVariableFullyQualified(full_name);
      if (!var)
        ARCANE_FATAL("Variable '{0}' not found");
      return var->itemGroup();
    }
   private:
    IVariableMng* m_variable_mng;
  };

 public:

  explicit ArcaneBasicVerifierService2(const ServiceBuildInfo& sbi)
  : VerifierService(sbi)
  {
  }
  ~ArcaneBasicVerifierService2()
  {
  }
    
 public:

  void build() override {}

  void writeReferenceFile() override
  {
    ISubDomain* sd = subDomain();
    IParallelMng* pm = sd->parallelMng();
    IVariableMng* vm = sd->variableMng();
    _computeFullFileName(false);
    String dir_name = platform::getFileDirName(m_full_file_name);
    platform::recursiveCreateDirectory(m_full_file_name);
    auto open_mode = BasicReaderWriterCommon::OpenModeTruncate;
    // Pour l'instant utilise la version 1
    // A partir de janvier 2019, il est possible d'utiliser la version 2
    // car le comparateur C# supporte cette version.
    Int32 version = 1;
    bool want_parallel = pm->isParallel();
    ScopedPtrT<BasicWriter> verif(new BasicWriter(sd->application(),pm,m_full_file_name,
                                                  open_mode,version,want_parallel));
    verif->initialize();

    // En parallèle, comme l'écriture nécessite des communications entre les sous-domaines,
    // il est indispensable que tous les PE aient les mêmes variables. On les filtre pour
    // garantir cela.
    VariableCollection used_variables = vm->usedVariables();
    if (pm->isParallel()){
      bool dump_not_common = true;
      VariableCollection filtered_variables = vm->utilities()->filterCommonVariables(pm,used_variables,dump_not_common);
      vm->writeVariables(verif.get(),filtered_variables);
    }
    else{
      vm->writeVariables(verif.get(),used_variables);
    }
  }

  void doVerifFromReferenceFile(bool parallel_sequential,bool compare_ghost) override
  {
    ISubDomain* sd = subDomain();
    IParallelMng* pm = sd->parallelMng();
    IVariableMng* vm = subDomain()->variableMng();
    ITraceMng* tm = sd->traceMng();
    _computeFullFileName(true);
    bool want_parallel = pm->isParallel();
    ScopedPtrT<BasicReader> reader(new BasicReader(sd->application(),pm,A_NULL_RANK,m_full_file_name,want_parallel));
    reader->initialize();
    GroupFinder group_finder(vm);
    reader->setItemGroupFinder(&group_finder);

    VariableList read_variables;
    _getVariables(read_variables,parallel_sequential);

    // En parallèle, comme la lecture nécessite des communications entre les sous-domaines,
    // il est indispensable que tous les PE aient les mêmes variables. On les filtre pour
    // garantir cela.
    if (pm->isParallel()){
      IVariableMng* vm = sd->variableMng();
      bool dump_not_common = true;
      VariableCollection filtered_variables = vm->utilities()->filterCommonVariables(pm,read_variables,dump_not_common);
      read_variables = filtered_variables;
    }

    tm->info() << "Checking (" << m_full_file_name << ")";
    reader->beginRead(read_variables);
    _doVerif(reader.get(),read_variables,compare_ghost);
    reader->endRead();
  }

 private:

  String m_full_file_name;

 private:

  void _computeFullFileName(bool is_read)
  {
    ARCANE_UNUSED(is_read);
    StringBuilder s  = fileName();
    //m_full_file_name = fileName();
    const String& sub_dir = subDir();
    if (!sub_dir.empty()){
      s += "/";
      s += sub_dir;
    }
    m_full_file_name = s;
  }
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

ARCANE_REGISTER_SERVICE(ArcaneBasicVerifierService2,
                        ServiceProperty("ArcaneBasicVerifier2",ST_SubDomain),
                        ARCANE_SERVICE_INTERFACE(IVerifierService));

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

} // End namespace Arcane

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
