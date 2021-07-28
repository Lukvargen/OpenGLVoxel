#version 330 core
out vec4 FragColor;  

in float light;
  
void main()
{
    FragColor = mix(vec4(0.2f,0.6f,0.2f,1), vec4(0.0f,0.0f,0.0f,1.0f), 1.0- light);
   //FragColor = vec4(0.2f,0.6f,0.2f,1);
   // FragColor = vec4(light,0.0f,0.0f,1);
}