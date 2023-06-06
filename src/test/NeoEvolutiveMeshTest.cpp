// -*- tab-width: 2; indent-tabs-mode: nil; coding: utf-8-with-signature -*-
//-----------------------------------------------------------------------------
// Copyright 2000-2023 CEA (www.cea.fr) IFPEN (www.ifpenergiesnouvelles.com)
// See the top-level COPYRIGHT file for details.
// SPDX-License-Identifier: Apache-2.0
//-----------------------------------------------------------------------------
/*---------------------------------------------------------------------------*/
/* NeoEvolutiveMeshTest.cpp                        (C) 2000-2023             */
/*                                                                           */
/* First very basic mesh evolution test                                      */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

#include "neo/Neo.h"
#include "neo/Mesh.h"
#include "gtest/gtest.h"

//----------------------------------------------------------------------------/

static const std::string cell_family_name{ "CellFamily" };
static const std::string face_family_name{ "FaceFamily" };
static const std::string node_family_name{ "NodeFamily" };

//----------------------------------------------------------------------------/

void createMesh(Neo::Mesh& mesh) {
  auto& cell_family = mesh.addFamily(Neo::ItemKind::IK_Cell, cell_family_name);
  auto& node_family = mesh.addFamily(Neo::ItemKind::IK_Node, node_family_name);
  auto& face_family = mesh.addFamily(Neo::ItemKind::IK_Face, face_family_name);
  std::vector<Neo::utils::Int64> node_uids{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
  std::vector<Neo::utils::Int64> cell_uids{ 0, 1, 2, 3 };
  std::vector<Neo::utils::Int64> face_uids{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

  std::vector<Neo::utils::Real3> node_coords{ { 0, 0, -2 }, { 0, 2, -2 }, { 0, 2, -2 }, { 0, 3, -2 }, { 0, 4, -2 }, { 0, 5, -2 }, { 0, 0, -2 }, { 0, 2, -2 }, { 0, 2, -2 }, { 0, 3, -2 }, { 0, 4, -2 }, { 0, 5, -2 } };

  std::vector<Neo::utils::Int64> cell_nodes{ 0, 1, 7, 6,
                                             2, 3, 9, 8,
                                             3, 4, 10, 9,
                                             4, 5, 11, 10 };

  std::vector<Neo::utils::Int64> face_nodes{ 6, 7, 8, 9, 9, 10, 10, 11, 1, 7, 2, 8, 0, 1, 2, 3, 3, 4, 4, 5 };

  auto added_cells = Neo::FutureItemRange{};
  auto added_nodes = Neo::FutureItemRange{};
  auto added_faces = Neo::FutureItemRange{};
  mesh.scheduleAddItems(cell_family, cell_uids, added_cells);
  mesh.scheduleAddItems(node_family, node_uids, added_nodes);
  mesh.scheduleAddItems(face_family, face_uids, added_faces);
  mesh.scheduleSetItemCoords(node_family, added_nodes, node_coords);
  auto nb_node_per_cell = 4;
  mesh.scheduleAddConnectivity(cell_family, added_cells, node_family, nb_node_per_cell, cell_nodes, "cell_to_nodes");
  auto nb_node_per_face = 2;
  mesh.scheduleAddConnectivity(face_family, added_faces, node_family, nb_node_per_face, face_nodes, "face_to_nodes");
  auto valid_mesh_state =
  mesh.applyScheduledOperations(); // retourner un objet qui dévérouille la range
  auto new_cells = added_cells.get(valid_mesh_state);
  auto new_nodes = added_nodes.get(valid_mesh_state);
  auto new_faces = added_faces.get(valid_mesh_state);
  std::cout << "Added cells range after applyAlgorithms: " << new_cells;
  std::cout << "Added nodes range after applyAlgorithms: " << new_nodes;
  std::cout << "Added faces range after applyAlgorithms: " << new_faces;
}

//----------------------------------------------------------------------------/

TEST(EvolutiveMeshTest, AddCells) {
  auto mesh = Neo::Mesh{ "evolutive_neo_mesh" };
  createMesh(mesh);
}

//----------------------------------------------------------------------------/

TEST(EvolutiveMeshTest, MoveNodes) {
  std::cout << "Move node test " << std::endl;
  auto mesh = Neo::Mesh{ "evolutive_neo_mesh" };
  createMesh(mesh);
  auto& node_family = mesh.findFamily(Neo::ItemKind::IK_Node, node_family_name);
  std::vector<Neo::utils::Int64> moved_node_uids{ 6, 7, 8, 9, 10, 11 };
  std::vector<Neo::utils::Real3> node_coords{ { 0, 0, -1 }, { 0, 1.5, -1 }, { 0, 1.5, -1 }, { 0, 2.7, -1 }, { 0, 3.85, -1 }, { 0, 5, -1 } };
  mesh.scheduleMoveNodes(node_family, moved_node_uids, node_coords);
  mesh.applyScheduledOperations();
  // Todo check new node coords
  auto moved_nodes = Neo::ItemRange{ Neo::ItemLocalIds::getIndexes(node_family._lidProp()[moved_node_uids]) };
  auto node_coords_new = mesh.getItemCoordProperty(node_family).view(moved_nodes);
  EXPECT_TRUE(std::equal(node_coords_new.begin(), node_coords_new.end(), node_coords.begin(),
                         [](Neo::utils::Real3 const& a, Neo::utils::Real3 const& b) { return a == b; }));
}

//----------------------------------------------------------------------------/

TEST(EvolutiveMeshTest, RemoveCells) {
  std::cout << "Remove cells test " << std::endl;
  auto mesh = Neo::Mesh{ "evolutive_neo_mesh" };
  createMesh(mesh);
  // add a connectivity to cell

  auto node2cells_con_name = node_family_name + "to" + cell_family_name + "_connectivity";
  std::vector<Neo::utils::Int64> node_to_cell{ 0, 0, 1, 1, 2, 3, 0, 0, 1, 2, 2, 3 };
  auto& cell_family = mesh.findFamily(Neo::ItemKind::IK_Cell, cell_family_name);
  auto& node_family = mesh.findFamily(Neo::ItemKind::IK_Node, node_family_name);
  mesh.scheduleAddConnectivity(node_family, node_family.all(), cell_family, 1,
                               node_to_cell, node2cells_con_name);
  mesh.applyScheduledOperations();
  // Remove cell 0, 1 and 2
  std::vector<Neo::utils::Int64> removed_cells{ 0, 1, 2 };
  mesh.scheduleRemoveItems(cell_family, removed_cells);
  // update connectivity must be donne automatically to check
  mesh.applyScheduledOperations();
  auto node2cells = node_family.getConcreteProperty<Neo::ArrayProperty<Neo::utils::Int32>>(node2cells_con_name);
  // compute a reference connectivity : replace removed cells by null lid
  std::fill(node_to_cell.begin(), node_to_cell.end(), Neo::utils::NULL_ITEM_LID);
  node_to_cell[5] = 3;
  node_to_cell[11] = 3;
  EXPECT_TRUE(std::equal(node2cells.view().begin(), node2cells.view().end(), node_to_cell.begin()));
}
