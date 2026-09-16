    // extract upper 3x3 from a mat4x4
    "fn mat3FromMat4(m:mat4x4<f32>)->mat3x3<f32>\n"+
    "{\n"+
    "return(mat3x3<f32>(m[0].xyz, m[1].xyz, m[2].xyz));\n"+
    "}\n"+
    // *******************************************************************


    // inverse
    "fn inverse3x3(m:mat3x3<f32>)->mat3x3<f32>\n"+
    "{\n"+
    "let determinant = m[0][0] * (m[1][1]*m[2][2] - m[2][1]*m[1][2]) - "+
    "                  m[1][0] * (m[0][1]*m[2][2] - m[2][1]*m[0][2]) + "+
    "                  m[2][0] * (m[0][1]*m[1][2] - m[1][1]*m[0][2]);\n"+
    "let inverse_determinant = 1.0 / determinant;\n"+
    "let column0 = vec3<f32>\n"+
    "(\n"+
    "    (m[1][1]*m[2][2] - m[2][1]*m[1][2]) * inverse_determinant,\n"+
    "    (m[2][1]*m[0][2] - m[0][1]*m[2][2]) * inverse_determinant,\n"+
    "    (m[0][1]*m[1][2] - m[1][1]*m[0][2]) * inverse_determinant"+
    ");\n"+
    "let column1 = vec3<f32>\n"+
    "(\n"+
    "    (m[2][0]*m[1][2] - m[1][0]*m[2][2]) * inverse_determinant,\n"+
    "    (m[0][0]*m[2][2] - m[2][0]*m[0][2]) * inverse_determinant,\n"+
    "    (m[1][0]*m[0][2] - m[0][0]*m[1][2]) * inverse_determinant"+
    ");\n"+
    "let column2 = vec3<f32>\n"+
    "(\n"+
    "    (m[1][0]*m[2][1] - m[2][0]*m[1][1]) * inverse_determinant,\n"+
    "    (m[2][0]*m[0][1] - m[0][0]*m[2][1]) * inverse_determinant,\n"+
    "    (m[0][0]*m[1][1] - m[1][0]*m[0][1]) * inverse_determinant"+
    ");\n"+
    "return(mat3x3<f32>(column0, column1, column2));\n"+
    "}\n";
    // *******************************************************************

