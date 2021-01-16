#pragma once

#include <vector>
#include "geom.h"
#include "../GfxCore/ray.h"

enum octreeRegion_t : uint8_t
{
	XN_YN_ZN = 0x0,
	XN_YN_ZP = 0x1,
	XN_YP_ZN = 0x2,
	XN_YP_ZP = 0x3,
	XP_YN_ZN = 0x4,
	XP_YN_ZP = 0x5,
	XP_YP_ZN = 0x6,
	XP_YP_ZP = 0x7,
	REGION_COUNT
};


template<typename T>
class Octree
{
public:

	Octree()
	{
		height = 0;
		bitSet = 0;
		parent = nullptr;
	}

	Octree( const vec3d& _min, const vec3d& _max )
	{
		height = 0;
		bitSet = 0;
		parent = nullptr;
		aabb.Expand( _min );
		aabb.Expand( _max );
	}

	bool Insert( const AABB& bounds, const T& item )
	{
		if( aabb.Inside( bounds ) )
		{
			bool inChildNode = false;

			if( !HasChildren() )
			{
				Split(); // For now just split the node if it's inside
			}

			const uint32_t childCnt = children.size();
			for( uint32_t i = 0; i < childCnt; ++i )
			{
				if( children[ i ].Insert( bounds, item ) )
				{
					inChildNode = true;
					break;
				}
			}

			if( !inChildNode )
			{
				items.push_back( item );
			}

			return true;
		}
		return false;
	}

	// TODO: move out of tree
	bool Intersect( const Ray& ray, std::vector<T>& hitItems ) const
	{
		double tNear;
		double tFar;

		if ( aabb.Intersect( ray, tNear, tFar ) )
		{
			const uint32_t childCnt = children.size();
			for ( uint32_t i = 0; i < childCnt; ++i )
			{
				children[ i ].Intersect( ray, hitItems );
			}

			const uint32_t itemCnt = items.size();
			for ( uint32_t itemIx = 0; itemIx < itemCnt; ++itemIx )
			{
				hitItems.push_back( items[ itemIx ] );
			}

			return true;
		}
		return false;
	}

	void IncreaseHeight()
	{
		height += 1;

		if( parent != nullptr )
		{
			parent->IncreaseHeight();
		}
	}

	bool HasChild( const octreeRegion_t region ) const
	{
		const uint8_t regionMask = ( 1 << region );
		return ( ( bitSet & regionMask ) != 0 );
	}

	bool HasChildren() const
	{
		return ( children.size() > 0 );
	}

	void Split()
	{
		// Can only 0 or 8 children currently
		if( children.size() >= REGION_COUNT )
		{
			return;
		}

		children.reserve( 8 );

		for( uint8_t i = 0; i < REGION_COUNT; ++i )
		{
			AddChild( (octreeRegion_t)i );
		}

		IncreaseHeight();
	}

	void SplitChild( const octreeRegion_t region )
	{
		if ( HasChild( region ) )
		{
			children[ region ].Split();
		}
	}

	AABB GetAABB() const
	{
		return aabb;
	}

private:
	void AddChild( const octreeRegion_t region )
	{
		const vec3d halfDist = 0.5 * ( aabb.max - aabb.min );
		
		const vec3d mask = vec3d(	( region & 0x04 ) ? 1.0 : 0.0,
									( region & 0x02 ) ? 1.0 : 0.0,
									( region & 0x01 ) ? 1.0 : 0.0 );

		const vec3d offset = vec3d( mask[ 0 ] * halfDist[ 0 ], mask[ 1 ] * halfDist[ 1 ], mask[ 2 ] * halfDist[ 2 ] );
		const vec3d nodeMin = aabb.min + offset;
		const vec3d nodeMax = nodeMin + halfDist;
		
		children.push_back( Octree( nodeMin, nodeMax ) );
		children.back().parent = this;

		bitSet |= ( 1 << region );
	}

	AABB				aabb;
	Octree<T>*			parent;
	uint32_t			height;
	uint8_t				bitSet;

	// TODO: make private
public:
	std::vector<T>				items;
	std::vector< Octree<T> >	children;
};