typedef float4 point;
typedef float4 vector;
typedef float4 color;
typedef float4 sphere;

vector
Bounce( vector in, vector n )
{
	vector out = in - (float4)2.*n*dot(in.xyz, n.xyz);
	out.w = 0.;
	return out;
}

vector
BounceSphere( point p, vector in, sphere s )
{
	vector n;
	n.xyz = fast_normalize( p.xyz - s.xyz );
	n.w = 0.;
	return Bounce( in, n );
}

bool
IsInsideSphere( point p, sphere s )
{
	float r = fast_length( p.xyz - s.xyz );
	return  ( r < s.w );
}


kernel
void
Particle( global point *dPobj, global vector *dVel, global color *dCobj )
{
	const float4 G       = (float4) ( 0., -9.8, 0., 0. );
	const float  DT      = 0.1;
	const sphere Sphere1 = (sphere)( -100., -800., 0.,  600. );
	const sphere Sphere2 = (sphere)(-100., -3000., 0., 2000);
	int gid = get_global_id( 0 );

	point  p = dPobj[gid];
	vector v = dVel[gid];
	color c = dCobj[gid];

	point  pp = p + v*DT + (float4)(.5*DT*DT)*G;
	vector vp = v + G*DT;

	/*c.x = pp.x;
	c.y = pp.y;
	c.z = pp.z;
	c.w = 1.;*/

	c.x = vp.x + pp.x;
	c.y = vp.y + pp.y;
	c.z = vp.z + pp.z;
	c.w = 1.;

	pp.w = 1.;
	vp.w = 0.;

	if( IsInsideSphere( pp, Sphere1 ) ) {
		vp = BounceSphere( p, v, Sphere1 );
		pp = p + vp*DT + (float4)(.5*DT*DT)*G;
		c.x = 0.0f;
		c.y = 0.0f;
		c.z = 0.9f;
		c.w = 1.;
	}
	if (IsInsideSphere(pp, Sphere2)) {
		vp = BounceSphere(p, v, Sphere2);
		pp = p + vp * DT + (float4)(.5*DT*DT)*G;
		c.x = 0.9f;
		c.y = 0.0f;
		c.z = 0.0f;
		c.w = 1.;
	}

	dPobj[gid] = pp;
	dVel[gid]  = vp;
	dCobj[gid] = c;
}
