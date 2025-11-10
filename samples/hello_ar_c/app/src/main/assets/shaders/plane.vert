precision highp float;
precision highp int;

attribute vec3 a_Vertex;
varying vec2 v_TextureCoords;
varying float v_Alpha;
varying vec3 v_ViewPosition;
varying vec3 v_ScreenSpacePosition;

uniform mat4 u_ModelViewProjection;
uniform mat4 u_ModelView;
uniform mat4 u_Model;
uniform vec3 u_Normal;

void main() {
  // Vertex Z value is used as the alpha in this shader.
  v_Alpha = a_Vertex.z;

  vec4 local_pos = vec4(a_Vertex.x, 0.0, a_Vertex.y, 1.0);
  gl_Position = u_ModelViewProjection * local_pos;
  vec4 world_pos = u_Model * local_pos;

  // Construct two vectors that are orthogonal to the normal.
  // This arbitrary choice is not co-linear with either horizontal
  // or vertical plane normals.
  const vec3 arbitrary = vec3(1.0, 1.0, 0.0);
  vec3 vec_u = normalize(cross(u_Normal, arbitrary));
  vec3 vec_v = normalize(cross(u_Normal, vec_u));

  // Project vertices in world frame onto vec_u and vec_v.
  v_TextureCoords = vec2(dot(world_pos.xyz, vec_u), dot(world_pos.xyz, vec_v));

  v_ViewPosition = (u_ModelView * local_pos).xyz;
  v_ScreenSpacePosition = gl_Position.xyz / gl_Position.w;
}
