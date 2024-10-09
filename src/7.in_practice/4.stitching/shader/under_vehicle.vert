precision highp float;
attribute highp vec3 position;
attribute highp vec2 texture_coord_vert;
varying highp vec2 texture_coord_frag;
uniform highp float dx;
uniform highp float dy;
uniform highp float dtheta;
uniform highp float back_center_x;
uniform highp float back_center_y;

void main(){
    gl_PointSize = 1.0;
    // vec2 car_system_coord = vec2(texture_coord_vert.x - back_center_x, texture_coord_vert.y - back_center_y);
    // vec2 rotated_car_system_coord = vec2(car_system_coord.x * cos(-dtheta) - car_system_coord.y * sin(-dtheta),\
    // car_system_coord.y * cos(-dtheta) + car_system_coord.x * sin(-dtheta));
    // vec2 rotate_offset = rotated_car_system_coord - car_system_coord;

    // texture_coord_frag = texture_coord_vert + rotate_offset + vec2(dx, dy);
    texture_coord_frag = texture_coord_vert;
    gl_Position = vec4(position, 1.0)  * vec4(1.0, -1.0, 1.0, 1.0); 	
}
