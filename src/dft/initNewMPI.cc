#include "initRho.cc"
#include "initPseudo.cc"

#ifdef ENABLE_PERIODIC_BC
#include "initkPointData.cc"
#endif

//
//source file for dft class initializations
//
void exchangeMasterNodesList(std::set<unsigned int> & masterNodeIdSet,
			     std::set<unsigned int> & globalMasterNodeIdSet,
			     unsigned int numMeshPartitions,
			     const MPI_Comm & mpi_communicator)
{

  std::vector<int> localMasterNodeIdList;
  std::copy(masterNodeIdSet.begin(),
	    masterNodeIdSet.end(),
	    std::back_inserter(localMasterNodeIdList));

  int numberMasterNodesOnLocalProc = localMasterNodeIdList.size();

  int * masterNodeIdListSizes = new int[numMeshPartitions];

  MPI_Allgather(&numberMasterNodesOnLocalProc,
		1,
		MPI_INT,
		masterNodeIdListSizes,
		1,
		MPI_INT,
		mpi_communicator);

  int newMasterNodeIdListSize = std::accumulate(&(masterNodeIdListSizes[0]),
						&(masterNodeIdListSizes[numMeshPartitions]),
						0);

  std::vector<int> globalMasterNodeIdList(newMasterNodeIdListSize);

  int * mpiOffsets = new int[numMeshPartitions];

  mpiOffsets[0] = 0;

  for(int i = 1; i < numMeshPartitions; ++i)
    mpiOffsets[i] = masterNodeIdListSizes[i-1] + mpiOffsets[i-1];

  MPI_Allgatherv(&(localMasterNodeIdList[0]),
		 numberMasterNodesOnLocalProc,
		 MPI_INT,
		 &(globalMasterNodeIdList[0]),
		 &(masterNodeIdListSizes[0]),
		 &(mpiOffsets[0]),
		 MPI_INT,
		 mpi_communicator);


  for(int i = 0; i < globalMasterNodeIdList.size(); ++i)
    globalMasterNodeIdSet.insert(globalMasterNodeIdList[i]);

  delete [] masterNodeIdListSizes;
  delete [] mpiOffsets;
	    
  return;

}


void exchangeMasterNodeIdCoordinateMap(std::map<unsigned int, Point<3> > & masterNodeIdCoordinatesMap,
				       unsigned int numMeshPartitions,
				       const MPI_Comm & mpi_communicator)

{

  std::vector<double> localMasterNodeIdCoordinateMapList;

  std::map<unsigned int, Point<3> >::iterator iter;

  for(iter = masterNodeIdCoordinatesMap.begin(); iter!= masterNodeIdCoordinatesMap.end(); ++iter)
    {
      unsigned int masterNodeId = iter->first;
      localMasterNodeIdCoordinateMapList.push_back(masterNodeId);
      Point<3> & nodalCoor = iter->second;
      localMasterNodeIdCoordinateMapList.push_back(nodalCoor[0]);
      localMasterNodeIdCoordinateMapList.push_back(nodalCoor[1]);
      localMasterNodeIdCoordinateMapList.push_back(nodalCoor[2]);
    }

  int sizeMapOnLocalProc = localMasterNodeIdCoordinateMapList.size();

  int * mapSizes = new int[numMeshPartitions];

  MPI_Allgather(&sizeMapOnLocalProc,
		1,
		MPI_INT,
		mapSizes,
		1,
		MPI_INT,
		mpi_communicator);

  int newMapSize = std::accumulate(&(mapSizes[0]),
				   &(mapSizes[numMeshPartitions]),
				   0);

  std::vector<double> globalMapList(newMapSize);

  int * mpiOffsets = new int[numMeshPartitions];

  mpiOffsets[0] = 0;

  for(int i = 1; i < numMeshPartitions; ++i)
    mpiOffsets[i] = mapSizes[i-1] + mpiOffsets[i-1];

  MPI_Allgatherv(&(localMasterNodeIdCoordinateMapList[0]),
		 sizeMapOnLocalProc,
		 MPI_DOUBLE,
		 &(globalMapList[0]),
		 &(mapSizes[0]),
		 &(mpiOffsets[0]),
		 MPI_DOUBLE,
		 mpi_communicator);

  int sizeMasterNodeIdsList = globalMapList.size()/4;

  masterNodeIdCoordinatesMap.clear();

  for(int i = 0; i < sizeMasterNodeIdsList; ++i)
    {
      unsigned int masterNodeId = globalMapList[4*i];
      double nodalCoorX = globalMapList[4*i + 1];
      double nodalCoorY = globalMapList[4*i + 2];
      double nodalCoorZ = globalMapList[4*i + 3];
      Point<3> nodalCoor(nodalCoorX,nodalCoorY,nodalCoorZ);
      masterNodeIdCoordinatesMap[masterNodeId] = nodalCoor;
    }

}


void dftClass::init(){
  computing_timer.enter_section("dftClass setup");


#ifdef ENABLE_PERIODIC_BC
  //mark faces
  double halfxyzSpan = 3.8;
  typename parallel::distributed::Triangulation<3>::active_cell_iterator cell = triangulation.begin_active(), endc = triangulation.end();
  for (; cell!=endc; ++cell) 
    {
      for (unsigned int f=0; f < GeometryInfo<3>::faces_per_cell; ++f)
	{
	  const Point<3> face_center = cell->face(f)->center();
	  if (cell->face(f)->at_boundary())
	    {
	      if (std::abs(face_center[0]+halfxyzSpan)<1.0e-8)
		cell->face(f)->set_boundary_id(1);
	      else if (std::abs(face_center[0]-halfxyzSpan)<1.0e-8)
		cell->face(f)->set_boundary_id(2);
	      else if (std::abs(face_center[1]+halfxyzSpan)<1.0e-8)
		cell->face(f)->set_boundary_id(3);
	      else if (std::abs(face_center[1]-halfxyzSpan)<1.0e-8)
		cell->face(f)->set_boundary_id(4);
	      else if (std::abs(face_center[2]+halfxyzSpan)<1.0e-8)
		cell->face(f)->set_boundary_id(5);
	      else if (std::abs(face_center[2]-halfxyzSpan)<1.0e-8)
		cell->face(f)->set_boundary_id(6);
	    }
	}
    }

  //mark faces
  cell = triangulation.begin_active(), endc = triangulation.end();
  for (; cell!=endc; ++cell) 
    {
      for (unsigned int f=0; f < GeometryInfo<3>::faces_per_cell; ++f)
	{
	  const Point<3> face_center = cell->face(f)->center();
	  if (cell->face(f)->at_boundary())
	    {
	      if (std::abs(face_center[0]+halfxyzSpan)<1.0e-8)
		cell->face(f)->set_boundary_id(1);
	      else if (std::abs(face_center[0]-halfxyzSpan)<1.0e-8)
		cell->face(f)->set_boundary_id(2);
	      else if (std::abs(face_center[1]+halfxyzSpan)<1.0e-8)
		cell->face(f)->set_boundary_id(3);
	      else if (std::abs(face_center[1]-halfxyzSpan)<1.0e-8)
		cell->face(f)->set_boundary_id(4);
	      else if (std::abs(face_center[2]+halfxyzSpan)<1.0e-8)
		cell->face(f)->set_boundary_id(5);
	      else if (std::abs(face_center[2]-halfxyzSpan)<1.0e-8)
		cell->face(f)->set_boundary_id(6);
	    }
	}
    }

  pcout << "Done with Boundary Flags\n";
  std::vector<GridTools::PeriodicFacePair<typename parallel::distributed::Triangulation<3>::cell_iterator> > periodicity_vector;
  for (int i = 0; i < 3; ++i)
    {
      GridTools::collect_periodic_faces(triangulation, /*b_id1*/ 2*i+1, /*b_id2*/ 2*i+2,/*direction*/ i, periodicity_vector);
    }
  triangulation.add_periodicity(periodicity_vector);
  std::cout << "Periodic Facepairs: " << periodicity_vector.size() << std::endl;
#endif  
    
  //
  //initialize FE objects
  //
  dofHandler.distribute_dofs (FE);
  dofHandlerEigen.distribute_dofs (FEEigen);

  locally_owned_dofs = dofHandler.locally_owned_dofs ();
  DoFTools::extract_locally_relevant_dofs (dofHandler, locally_relevant_dofs);
  DoFTools::map_dofs_to_support_points(MappingQ1<3,3>(), dofHandler, d_supportPoints);

  locally_owned_dofsEigen = dofHandlerEigen.locally_owned_dofs ();
  DoFTools::extract_locally_relevant_dofs (dofHandlerEigen, locally_relevant_dofsEigen);
  DoFTools::map_dofs_to_support_points(MappingQ1<3,3>(), dofHandlerEigen, d_supportPointsEigen);

  //
  //Extract real and imag DOF indices from the global vector - this will be needed in XHX operation, etc.
  //
#ifdef ENABLE_PERIODIC_BC
  FEValuesExtractors::Scalar real(0); //For Eigen
  ComponentMask componentMaskForRealDOF = FEEigen.component_mask (real);
  std::vector<bool> selectedDofsReal(locally_owned_dofsEigen.n_elements(), false);
  DoFTools::extract_dofs(dofHandlerEigen, componentMaskForRealDOF, selectedDofsReal);
  std::vector<unsigned int> local_dof_indices(locally_owned_dofsEigen.n_elements());
  locally_owned_dofsEigen.fill_index_vector(local_dof_indices);
  for (unsigned int i = 0; i < locally_owned_dofsEigen.n_elements(); i++)
    {
      if (selectedDofsReal[i]) 
	{
	  local_dof_indicesReal.push_back(local_dof_indices[i]);
	}
      else
	{
	  local_dof_indicesImag.push_back(local_dof_indices[i]);
	}
    }
#endif


  
  pcout << "number of elements: "
	<< triangulation.n_global_active_cells()
	<< std::endl
	<< "number of degrees of freedom: " 
	<< dofHandler.n_dofs() 
	<< std::endl;

  //
  //write mesh to vtk file
  //
  DataOut<3> data_out;
  data_out.attach_dof_handler (dofHandler);
  data_out.build_patches ();
  std::ofstream output("mesh.vtu");
  data_out.write_vtu(output); 

  //
  //matrix free data structure
  //
  typename MatrixFree<3>::AdditionalData additional_data;
  additional_data.mpi_communicator = MPI_COMM_WORLD;
  additional_data.tasks_parallel_scheme = MatrixFree<3>::AdditionalData::partition_partition;

  //
  //constraints
  //

  //
  //hanging node constraints
  //
  constraintsNone.clear(); constraintsNoneEigen.clear();
  DoFTools::make_hanging_node_constraints (dofHandler, constraintsNone);
  DoFTools::make_hanging_node_constraints (dofHandlerEigen, constraintsNoneEigen);


#ifdef ENABLE_PERIODIC_BC
  std::vector<GridTools::PeriodicFacePair<typename DoFHandler<3>::cell_iterator> > periodicity_vector2, periodicity_vector2Eigen;
  for (int i = 0; i < 3; ++i)
    {
      GridTools::collect_periodic_faces(dofHandler, /*b_id1*/ 2*i+1, /*b_id2*/ 2*i+2,/*direction*/ i, periodicity_vector2);
      GridTools::collect_periodic_faces(dofHandlerEigen, /*b_id1*/ 2*i+1, /*b_id2*/ 2*i+2,/*direction*/ i, periodicity_vector2Eigen);
    }
  DoFTools::make_periodicity_constraints<DoFHandler<3> >(periodicity_vector2, constraintsNone);
  DoFTools::make_periodicity_constraints<DoFHandler<3> >(periodicity_vector2Eigen, constraintsNoneEigen);
  pcout << "Detected Periodic Face Pairs: " << constraintsNone.n_constraints() << std::endl;

  pcout<<"Size of ConstraintsNone: "<< constraintsNone.n_constraints()<<std::endl;

  //
  //modify constraintsNone to account for the bug in higher order nodes
  //
  ConstraintMatrix constraintsTemp(constraintsNone); constraintsNone.clear();
  std::set<unsigned int> masterNodes;
  std::set<unsigned int> globalMasterNodeIdSet;

  double periodicPrecision=1.0e-8;
  //fill all masters
  for(types::global_dof_index i = 0; i < dofHandler.n_dofs(); ++i){
    if(locally_relevant_dofs.is_element(i)){
      if(constraintsTemp.is_constrained(i)){
	if (constraintsTemp.is_identity_constrained(i)){
	  Point<3> p = d_supportPoints.find(i)->second;
	  unsigned int masterNode=(*constraintsTemp.get_constraint_entries(i))[0].first;
	  masterNodes.insert(masterNode);
	}
      }
    }
  }

  exchangeMasterNodesList(masterNodes,
			  globalMasterNodeIdSet,
			  n_mpi_processes,
			  mpi_communicator);

  std::map<unsigned int, Point<3> > masterNodeIdCoordinatesMap;

  for(std::set<unsigned int>::iterator iter = globalMasterNodeIdSet.begin(); iter != globalMasterNodeIdSet.end(); ++iter)
    {
      if(locally_relevant_dofs.is_element(*iter))
	{
	  masterNodeIdCoordinatesMap[*iter] = d_supportPoints.find(*iter)->second;
	}
    }
      
  exchangeMasterNodeIdCoordinateMap(masterNodeIdCoordinatesMap,
				    n_mpi_processes,
				    mpi_communicator);

  //
  //fix wrong master map
  //
  for(types::global_dof_index i = 0; i < dofHandler.n_dofs(); ++i){
    if(locally_relevant_dofs.is_element(i)){
      if(constraintsTemp.is_constrained(i)){
	if (constraintsTemp.is_identity_constrained(i)){
	  Point<3> p=d_supportPoints.find(i)->second;
	  unsigned int masterNode=(*constraintsTemp.get_constraint_entries(i))[0].first;
	  unsigned int count=0, index=0;
	  for (unsigned int k=0; k<3; k++){
	    if (std::abs(std::abs(p[k])-halfxyzSpan)<periodicPrecision) {
	      count++;
	      index=k;
	    }
	  }
	  if (count==1){
	    //Point<3> q=d_supportPoints.find(masterNode)->second;
	    Point<3> q = masterNodeIdCoordinatesMap[masterNode];
	    unsigned int l=1, m=2;
	    if (index==1){l=0; m=2;}
	    else if (index==2){l=0; m=1;} 
	    if (!((std::abs(p[l]-q[l])<periodicPrecision) and (std::abs(p[m]-q[m])<periodicPrecision))){
	      bool foundNewMaster=false;
	      for (std::set<unsigned int>::iterator it=masterNodes.begin(); it!=masterNodes.end(); ++it){
		//q = d_supportPoints.find(*it)->second;
		q = masterNodeIdCoordinatesMap[*it];
		if (((std::abs(p[l]-q[l])<periodicPrecision) and (std::abs(p[m]-q[m])<periodicPrecision))){
		  constraintsNone.add_line(i);
		  constraintsNone.add_entry(i, *it, 1.0);
		  foundNewMaster=true;
		  break;
		}
	      }
	      if (!foundNewMaster){
		pcout << "\nError: Didnot find a replacement master node for a wrong master-slave periodic pair\n";
		exit(-1);
	      }
	    }
	    else{
	      constraintsNone.add_line(i);
	      constraintsNone.add_entry(i, masterNode, 1.0);
	    }
	  }
	  else{
	    constraintsNone.add_line(i);
	    constraintsNone.add_entry(i, masterNode, 1.0);
	  }
	}
      }
    }
  }
  constraintsNone.close();
  std::cout<<"Size of ConstraintsNone after fixing periodicity: "<< constraintsNone.n_constraints()<<std::endl;
   //
  //modify constraintsNoneEigen to account for the bug in higher order nodes
  //
  ConstraintMatrix constraintsTempEigen(constraintsNoneEigen); constraintsNoneEigen.clear();
  std::set<unsigned int> masterNodesEigen;
  //fill all masters
  for(types::global_dof_index i = 0; i <dofHandlerEigen.n_dofs(); ++i){
    if(locally_relevant_dofsEigen.is_element(i)){
      if(constraintsTempEigen.is_constrained(i)){
	if (constraintsTempEigen.is_identity_constrained(i)){
	  Point<3> p=d_supportPointsEigen.find(i)->second;
	  unsigned int masterNode=(*constraintsTempEigen.get_constraint_entries(i))[0].first;
	  masterNodesEigen.insert(masterNode);
	}
      }
    }
  }

  //
  //fix wrong master map
  //
  for(types::global_dof_index i = 0; i <dofHandlerEigen.n_dofs(); ++i){
    if(locally_relevant_dofsEigen.is_element(i)){
      if(constraintsTempEigen.is_constrained(i)){
	if (constraintsTempEigen.is_identity_constrained(i)){
	  Point<3> p=d_supportPointsEigen.find(i)->second;
	  unsigned int masterNode=(*constraintsTempEigen.get_constraint_entries(i))[0].first;
	  unsigned int count=0, index=0;
	  for (unsigned int k=0; k<3; k++){
	    if (std::abs(std::abs(p[k])-halfxyzSpan)<periodicPrecision) {
	      count++;
	      index=k;
	    }
	  }
	  if (count==1){
	    Point<3> q=d_supportPointsEigen.find(masterNode)->second;
	    unsigned int l=1, m=2;
	    if (index==1){l=0; m=2;}
	    else if (index==2){l=0; m=1;} 
	    if (!((std::abs(p[l]-q[l])<periodicPrecision) and (std::abs(p[m]-q[m])<periodicPrecision))){
	      bool foundNewMaster=false;
	      for (std::set<unsigned int>::iterator it=masterNodes.begin(); it!=masterNodes.end(); ++it){
		q=d_supportPointsEigen.find(*it)->second;
		if (((std::abs(p[l]-q[l])<periodicPrecision) and (std::abs(p[m]-q[m])<periodicPrecision))){
		  constraintsNoneEigen.add_line(i);
		  constraintsNoneEigen.add_entry(i, *it, 1.0);
		  foundNewMaster=true;
		  break;
		}
	      }
	      if (!foundNewMaster){
		pcout << "\nError: Didnot find a replacement master node for a wrong master-slave periodic pair\n";
		exit(-1);
	      }
	    }
	    else{
	      constraintsNoneEigen.add_line(i);
	      constraintsNoneEigen.add_entry(i, masterNode, 1.0);
	    }
	  }
	  else{
	    constraintsNoneEigen.add_line(i);
	    constraintsNoneEigen.add_entry(i, masterNode, 1.0);
	  }
	}
      }
    }
  }
  constraintsNoneEigen.close();
#else
  constraintsNone.close();
  constraintsNoneEigen.close();
#endif

  //
  //Zero Dirichlet BC constraints on the boundary of the domain
  //used for computing total electrostatic potential using Poisson problem
  //with (rho+b) as the rhs
  //
  d_constraintsForTotalPotential.clear ();  
  DoFTools::make_hanging_node_constraints (dofHandler, d_constraintsForTotalPotential);
#ifdef ENABLE_PERIODIC_BC
  locatePeriodicPinnedNodes();
#else
  VectorTools::interpolate_boundary_values (dofHandler, 0, ZeroFunction<3>(), d_constraintsForTotalPotential);
#endif
  d_constraintsForTotalPotential.close ();

#ifdef ENABLE_PERIODIC_BC 
  d_constraintsPeriodicWithDirichlet.clear();
  DoFTools::make_hanging_node_constraints (dofHandler, d_constraintsPeriodicWithDirichlet);
  d_constraintsPeriodicWithDirichlet.merge(d_constraintsForTotalPotential);
  d_constraintsPeriodicWithDirichlet.close();
  d_constraintsPeriodicWithDirichlet.merge(constraintsNone);
  d_constraintsPeriodicWithDirichlet.close();  
  std::cout<<"Updated Size of ConstraintsPeriodic with Dirichlet B.Cs: "<< d_constraintsPeriodicWithDirichlet.n_constraints()<<std::endl;
#endif
 
  

  //
  //push back into Constraint Matrices
  //
  d_constraintsVector.clear();
#ifdef ENABLE_PERIODIC_BC
  //d_constraintsVector.push_back(&d_constraintsPeriodicWithDirichlet); 
  d_constraintsVector.push_back(&constraintsNone); 
#else
  d_constraintsVector.push_back(&constraintsNone); 
#endif

  d_constraintsVector.push_back(&d_constraintsForTotalPotential);

  //
  //Dirichlet BC constraints on the boundary of fictitious ball
  //used for computing self-potential (Vself) using Poisson problem
  //with atoms belonging to a given bin
  //
  createAtomBins(d_constraintsVector);
 
 
  //
  //create matrix free structure
  //
  std::vector<const DoFHandler<3> *> dofHandlerVector; 
  
  for(int i = 0; i < d_constraintsVector.size(); ++i)
    dofHandlerVector.push_back(&dofHandler);
 
  dofHandlerVector.push_back(&dofHandlerEigen); //DofHandler For Eigen
  eigenDofHandlerIndex=dofHandlerVector.size()-1; //For Eigen
  d_constraintsVector.push_back(&constraintsNoneEigen); //For Eigen;

  std::vector<Quadrature<1> > quadratureVector; 
  quadratureVector.push_back(QGauss<1>(FEOrder+1)); 
  quadratureVector.push_back(QGaussLobatto<1>(FEOrder+1));  


  matrix_free_data.reinit(dofHandlerVector, d_constraintsVector, quadratureVector, additional_data);


  //
  //initialize eigen vectors
  //
  matrix_free_data.initialize_dof_vector(vChebyshev,eigenDofHandlerIndex);
  v0Chebyshev.reinit(vChebyshev);
  fChebyshev.reinit(vChebyshev);
  aj[0].reinit(vChebyshev); aj[1].reinit(vChebyshev); aj[2].reinit(vChebyshev);
  aj[3].reinit(vChebyshev); aj[4].reinit(vChebyshev);
  for (unsigned int i=0; i<eigenVectors[0].size(); ++i)
    {  
      PSI[i]->reinit(vChebyshev);
      tempPSI[i]->reinit(vChebyshev);
      tempPSI2[i]->reinit(vChebyshev);
      tempPSI3[i]->reinit(vChebyshev);
      tempPSI4[i]->reinit(vChebyshev);
    } 
  
  for(unsigned int kPoint = 0; kPoint < d_maxkPoints; ++kPoint)
    {
      for (unsigned int i=0; i<eigenVectors[kPoint].size(); ++i)
	{
	  eigenVectors[kPoint][i]->reinit(vChebyshev);
	  eigenVectorsOrig[kPoint][i]->reinit(vChebyshev);
	}
    }

  //
  //locate atom core nodes and also locate atom nodes in each bin 
  //
  locateAtomCoreNodes();

  
  //
  //initialize density 
  //
  initRho();

  //
  //Initialize libxc (exchange-correlation)
  //
  int exceptParamX, exceptParamC;

#ifdef xc_id
  #if   xc_id == 1
  exceptParamX = xc_func_init(&funcX,XC_LDA_X,XC_UNPOLARIZED);
  exceptParamC = xc_func_init(&funcC,XC_LDA_C_PZ,XC_UNPOLARIZED);
  #elif xc_id == 2
  exceptParamX = xc_func_init(&funcX,XC_LDA_X,XC_UNPOLARIZED);
  exceptParamC = xc_func_init(&funcC,XC_LDA_C_PW,XC_UNPOLARIZED);
  #elif xc_id == 3
  exceptParamX = xc_func_init(&funcX,XC_LDA_X,XC_UNPOLARIZED);
  exceptParamC = xc_func_init(&funcC,XC_LDA_C_VWN,XC_UNPOLARIZED);
  #elif xc_id == 4
  exceptParamX = xc_func_init(&funcX,XC_GGA_X_PBE,XC_UNPOLARIZED);
  exceptParamC = xc_func_init(&funcC,XC_GGA_C_PBE,XC_UNPOLARIZED);
  #elif xc_id > 4
  #error exchange correlation id <= 4 required. 
  #endif
#else
  exceptParamX = xc_func_init(&funcX,XC_LDA_X,XC_UNPOLARIZED);
  exceptParamC = xc_func_init(&funcC,XC_LDA_C_PZ,XC_UNPOLARIZED);
  if(exceptParamX != 0 || exceptParamC != 0){
    pcout<<"-------------------------------------"<<std::endl;
    pcout<<"Exchange or Correlation Functional not found"<<std::endl;
    pcout<<"-------------------------------------"<<std::endl;
    exit(-1);
  }
#endif

  //
  //initialize local pseudopotential
  //
  if(isPseudopotential)
    {
      initLocalPseudoPotential();
      initNonLocalPseudoPotential();
      computeSparseStructureNonLocalProjectors();
      computeElementalProjectorKets();
    }

 

  //
  //
  //
  computing_timer.exit_section("dftClass setup"); 

  //
  //initialize poisson and eigen problem related objects
  //
  poisson.init();
  eigen.init();
  
  //
  //initialize PSI
  //
  pcout << "reading initial guess for PSI\n";
  readPSI();
}