#!/usr/bin/env pike

int main(int argc, array(string) argv)
{
  string code = Stdio.File("stdin")->read();
  write("unsigned int arm_pxt_code[] = {\n");
  foreach(code/24.0, string l)
    write(sprintf("%{ 0x%:08x,%}\n",
		  column(map(l/4.0, array_sscanf, "%-4c"), 0)));
  write("};\n");
  return 0;
}
