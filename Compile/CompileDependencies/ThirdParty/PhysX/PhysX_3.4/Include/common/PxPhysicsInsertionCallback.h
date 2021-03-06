// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2018 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  


#ifndef PX_PHYSICS_PX_PHYSICS_INSERTION_CALLBACK
#define PX_PHYSICS_PX_PHYSICS_INSERTION_CALLBACK

#include "PxBase.h"

/** \addtogroup common
@{
*/

#if !PX_DOXYGEN
namespace physx
{
#endif

	/**

	\brief Callback interface that permits PxCooking to insert a
	TriangleMesh, HeightfieldMesh or ConvexMesh directly into PxPhysics without the need to store
	the cooking results into a stream.


	Using this is advised only if real-time cooking is required; using "offline" cooking and
	streams is otherwise preferred.

	The default PxPhysicsInsertionCallback implementation must be used. The PxPhysics
	default callback can be obtained using the PxPhysics::getPhysicsInsertionCallback().

	@see PxCooking PxPhysics
	*/
	class PxPhysicsInsertionCallback
	{
	public:
		PxPhysicsInsertionCallback()				{}		

		/**
		\brief Builds object (TriangleMesh, HeightfieldMesh or ConvexMesh) from given data in PxPhysics.		

		\param type Object type to build.
		\param data Object data
		\return PxBase Created object in PxPhysics.
		*/
		virtual PxBase* buildObjectFromData(PxConcreteType::Enum type, void* data) = 0;

	protected:
		virtual ~PxPhysicsInsertionCallback()		{}
	};


#if !PX_DOXYGEN
} // namespace physx
#endif

/** @} */
#endif
