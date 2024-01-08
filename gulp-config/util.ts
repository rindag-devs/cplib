export function stripExtension(str: string): string {
  return str.substring(0, str.lastIndexOf("."));
}
