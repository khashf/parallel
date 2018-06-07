typedef float4 point;
typedef float4 vector;
typedef float4 color;
typedef float4 sphere;


vector
Bounce( vector in, vector n )
{
	float mag = 2.*dot(in.xyz, n.xyz);
	vector out = (float4) (mag*n.x, mag*n.y, mag*n.z, 0.);
	//vector out = in - 2.*n*dot(in.xyz, n.xyz);
	//out.w = 0.;
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
	int gid = get_global_id( 0 );

	point  p = dPobj[gid];
	vector v = dVel[gid];
	color c = dCobj[gid];

	//point  pp = p + v*DT + .5*DT*DT*G;
	//point  pp = p + (point) (DT*v.x, DT*v.y, DT*v.z, 1.) + ;
	float k = .5*DT*DT;
	point  pp = p + v * DT + (float4) (k*G.x, k*G.y, k*G.z, 1.);
	vector vp = v + G*DT;
	pp.w = 1.;
	vp.w = 0.;

	if( IsInsideSphere( pp, Sphere1 ) )
	{
		vp = BounceSphere( p, v, Sphere1 );
		//pp = p + vp*DT + .5*DT*DT*G;
		float k = .5*DT*DT;
		pp = p + vp * DT + (float4) (k*G.x, k*G.y, k*G.z, 1.);
		dCobj[gid] = (float4) (1., 0., 0., 0.);
	}

	dPobj[gid] = pp;
	dVel[gid]  = vp;
}
