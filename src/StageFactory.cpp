/******************************************************************************
* Copyright (c) 2011, Michael P. Gerlek (mpg@flaxen.com)
*
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following
* conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in
*       the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of Hobu, Inc. or Flaxen Geo Consulting nor the
*       names of its contributors may be used to endorse or promote
*       products derived from this software without specific prior
*       written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
* AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
* OF SUCH DAMAGE.
****************************************************************************/

#include <pdal/StageFactory.hpp>
#include <pdal/PluginManager.hpp>
#include <pdal/util/FileUtils.hpp>

// filters
#include <attribute/AttributeFilter.hpp>
#include <chipper/ChipperFilter.hpp>
#include <colorization/ColorizationFilter.hpp>
#include <crop/CropFilter.hpp>
#include <decimation/DecimationFilter.hpp>
#include <divider/DividerFilter.hpp>
#include <ferry/FerryFilter.hpp>
#include <merge/MergeFilter.hpp>
#include <mortonorder/MortonOrderFilter.hpp>
#include <range/RangeFilter.hpp>
#include <reprojection/ReprojectionFilter.hpp>
#include <sort/SortFilter.hpp>
#include <splitter/SplitterFilter.hpp>
#include <stats/StatsFilter.hpp>
#include <transformation/TransformationFilter.hpp>

// readers
#include <bpf/BpfReader.hpp>
#include <faux/FauxReader.hpp>
#include <gdal/GDALReader.hpp>
#include <ilvis2/Ilvis2Reader.hpp>
#include <las/LasReader.hpp>
#include <optech/OptechReader.hpp>
#include <buffer/BufferReader.hpp>
#include <ply/PlyReader.hpp>
#include <qfit/QfitReader.hpp>
#include <sbet/SbetReader.hpp>
#include <terrasolid/TerrasolidReader.hpp>
#include <text/TextReader.hpp>
#include <tindex/TIndexReader.hpp>

// writers
#include <bpf/BpfWriter.hpp>
#include <las/LasWriter.hpp>
#include <ply/PlyWriter.hpp>
#include <sbet/SbetWriter.hpp>
#include <derivative/DerivativeWriter.hpp>
#include <text/TextWriter.hpp>
#include <null/NullWriter.hpp>

#include <sstream>
#include <string>
#include <stdio.h> // for funcptr

namespace pdal
{

std::string StageFactory::inferReaderDriver(const std::string& filename)
{
    // filename may actually be a greyhound uri + pipelineId
    std::string http = filename.substr(0, 4);
    if (Utils::iequals(http, "http"))
        return "readers.greyhound";

    std::string ext = FileUtils::extension(filename);
    std::map<std::string, std::string> drivers;
    drivers["bin"] = "readers.terrasolid";
    drivers["bpf"] = "readers.bpf";
    drivers["csd"] = "readers.optech";
    drivers["greyhound"] = "readers.greyhound";
    drivers["icebridge"] = "readers.icebridge";
    drivers["las"] = "readers.las";
    drivers["laz"] = "readers.las";
    drivers["nitf"] = "readers.nitf";
    drivers["nsf"] = "readers.nitf";
    drivers["ntf"] = "readers.nitf";
    drivers["pcd"] = "readers.pcd";
    drivers["ply"] = "readers.ply";
    drivers["qi"] = "readers.qfit";
    drivers["rxp"] = "readers.rxp";
    drivers["sbet"] = "readers.sbet";
    drivers["sqlite"] = "readers.sqlite";
    drivers["sid"] = "readers.mrsid";
    drivers["tindex"] = "readers.tindex";
    drivers["txt"] = "readers.text";
    drivers["h5"] = "readers.icebridge";

    if (ext == "")
        return "";
    ext = ext.substr(1, ext.length()-1);
    if (ext == "")
        return "";

    ext = Utils::tolower(ext);
    std::string driver = drivers[ext];
    return driver; // will be "" if not found
}


std::string StageFactory::inferWriterDriver(const std::string& filename)
{
    std::string ext = Utils::tolower(FileUtils::extension(filename));

    std::map<std::string, std::string> drivers;
    drivers["bpf"] = "writers.bpf";
    drivers["csv"] = "writers.text";
    drivers["json"] = "writers.text";
    drivers["las"] = "writers.las";
    drivers["laz"] = "writers.las";
    drivers["mat"] = "writers.matlab";
    drivers["ntf"] = "writers.nitf";
    drivers["pcd"] = "writers.pcd";
    drivers["pclviz"] = "writers.pclvisualizer";
    drivers["ply"] = "writers.ply";
    drivers["sbet"] = "writers.sbet";
    drivers["derivative"] = "writers.derivative";
    drivers["sqlite"] = "writers.sqlite";
    drivers["txt"] = "writers.text";
    drivers["xyz"] = "writers.text";

    if (Utils::iequals(filename, "STDOUT"))
        return drivers["txt"];

    if (ext == "")
        return drivers["txt"];
    ext = ext.substr(1, ext.length()-1);
    if (ext == "")
        return drivers["txt"];

    ext = Utils::tolower(ext);
    std::string driver = drivers[ext];
    return driver; // will be "" if not found
}


pdal::Options StageFactory::inferWriterOptionsChanges(
    const std::string& filename)
{
    std::string ext = FileUtils::extension(filename);
    ext = Utils::tolower(ext);
    Options options;

    if (Utils::iequals(ext,".laz"))
        options.add("compression", true);

    if (Utils::iequals(ext, ".pcd") &&
        PluginManager::createObject("writers.pcd"))
    {
        options.add("format","PCD");
    }

    return options;
}


StageFactory::StageFactory(bool no_plugins)
{
    if (!no_plugins)
    {
        PluginManager::loadAll(PF_PluginType_Filter | PF_PluginType_Reader |
            PF_PluginType_Writer);
    }

    // filters
    PluginManager::initializePlugin(AttributeFilter_InitPlugin);
    PluginManager::initializePlugin(ChipperFilter_InitPlugin);
    PluginManager::initializePlugin(ColorizationFilter_InitPlugin);
    PluginManager::initializePlugin(CropFilter_InitPlugin);
    PluginManager::initializePlugin(DecimationFilter_InitPlugin);
    PluginManager::initializePlugin(DividerFilter_InitPlugin);
    PluginManager::initializePlugin(FerryFilter_InitPlugin);
    PluginManager::initializePlugin(MergeFilter_InitPlugin);
    PluginManager::initializePlugin(MortonOrderFilter_InitPlugin);
    PluginManager::initializePlugin(RangeFilter_InitPlugin);
    PluginManager::initializePlugin(ReprojectionFilter_InitPlugin);
    PluginManager::initializePlugin(SortFilter_InitPlugin);
    PluginManager::initializePlugin(SplitterFilter_InitPlugin);
    PluginManager::initializePlugin(StatsFilter_InitPlugin);
    PluginManager::initializePlugin(TransformationFilter_InitPlugin);

    // readers
    PluginManager::initializePlugin(BpfReader_InitPlugin);
    PluginManager::initializePlugin(FauxReader_InitPlugin);
    PluginManager::initializePlugin(GDALReader_InitPlugin);
    PluginManager::initializePlugin(Ilvis2Reader_InitPlugin);
    PluginManager::initializePlugin(LasReader_InitPlugin);
    PluginManager::initializePlugin(OptechReader_InitPlugin);
    PluginManager::initializePlugin(PlyReader_InitPlugin);
    PluginManager::initializePlugin(QfitReader_InitPlugin);
    PluginManager::initializePlugin(SbetReader_InitPlugin);
    PluginManager::initializePlugin(TerrasolidReader_InitPlugin);
    PluginManager::initializePlugin(TextReader_InitPlugin);
    PluginManager::initializePlugin(TIndexReader_InitPlugin);

    // writers
    PluginManager::initializePlugin(BpfWriter_InitPlugin);
    PluginManager::initializePlugin(LasWriter_InitPlugin);
    PluginManager::initializePlugin(PlyWriter_InitPlugin);
    PluginManager::initializePlugin(SbetWriter_InitPlugin);
    PluginManager::initializePlugin(DerivativeWriter_InitPlugin);
    PluginManager::initializePlugin(TextWriter_InitPlugin);
    PluginManager::initializePlugin(NullWriter_InitPlugin);
}


Stage *StageFactory::createStage(std::string const& stage_name)
{
    static_assert(0 < sizeof(Stage), "");
    Stage *s = static_cast<Stage*>(PluginManager::createObject(stage_name));
    if (s)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_ownedStages.push_back(std::unique_ptr<Stage>(s));
    }
    return s;
}


void StageFactory::destroyStage(Stage *s)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto it = m_ownedStages.begin(); it != m_ownedStages.end(); ++it)
    {
        if (s == it->get())
        {
            m_ownedStages.erase(it);
            break;
        }
    }
}

} // namespace pdal
