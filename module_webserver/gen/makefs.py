import sys
import os
import re
import mimetypes
import ConfigParser

def get_id(root, name=None):
    root = root.replace('/','_')
    root = root.replace('\\','_')
    if name:
        name = name.replace('.','_dot_')
        return '_'+root+'_'+name
    else:
        return '_'+root

def get_next_ptr(root, fs, i):
    if i < len(fs)-1:
        return '&' + get_id(root,fs[i+1])
    else:
        return 'NULL'

def to_char_array(s,add_null = True):
    chars = [str(ord(c)) for c in s]
    if add_null:
        chars.append('0')
    return '{' + ','.join(chars) + '}'



header = 'HTTP/1.0 200 OK\nServer: XMOS\nContent-type: %s\n\n'

fs_type_template = 0
fs_type_binary = 1

crlf = str(chr(13)) + str(chr(10))
lf = str(chr(10))
cr = str(chr(13))

def normalize_line_endings(s):
    s = s.replace(crlf, lf)
    s = s.replace(lf, crlf)
    return s


def process_file(path):
    global dyn_exprs, dyn_expr_count, decls
    (typ,_) = mimetypes.guess_type(path)
    if not typ:
        type = 'application/octet-stream'

    if re.match('text/.*',typ):
        fs_type = fs_type_template
    else:
        fs_type = fs_type_binary

    f = open(path,"rb")
    bytes = []
    while True:
        byte = f.read(1)
        if byte == "":
            break
        bytes.append(byte)

    bytes = ''.join(bytes)

    hdr = header%typ

    hdr = normalize_line_endings(hdr)

    out = hdr
    length = len(hdr)

    if fs_type == fs_type_binary:
        out += bytes
        length += len(bytes)
    else:
        bytes = normalize_line_endings(bytes)

        fchunk = False
        for chunk in re.split('{%|%}',bytes):
            if fchunk:
                chunk = chunk.strip()
                if chunk in dyn_exprs:
                    index = dyn_exprs[chunk]
                else:
                    index = dyn_expr_count
                    dyn_exprs[chunk] = index
                    dyn_expr_count += 1
                out += ''.join([chr(255), chr(index)])
                length += 2
            else:
                out += chunk
                length += len(chunk)

            fchunk = not fchunk

    return (length, fs_type, out)




def traverse(root, next_ptr = 'NULL',name = ''):
    global binfile, binindex
    if os.path.exists(root) and os.path.isdir(root):
        fs = os.listdir(root)
    else:
        fs = []

    subdirs = [ f for f in fs if os.path.isdir(os.path.join(root,f)) ]
    fs = [ f for f in fs if not os.path.isdir(os.path.join(root,f)) ]

    fs = [f for f in fs if not re.match('.*webserver.conf',f)]

    sym = get_id(root,None)
    decl = 'fs_dir_t %s = {%s, %s, %s, %s};' % (sym,
                                                next_ptr,
                                                get_next_ptr(root,subdirs,-1),
                                                get_next_ptr(root,fs,-1),
                                                to_char_array(name))
    decls.append(decl)

    for i in range(len(fs)):
        next_ptr = get_next_ptr(root,fs,i)
        sym = get_id(root, fs[i])
        name = fs[i]
        (length, fs_type, data) = process_file(os.path.join(root,fs[i]))

        if is_flash_fs:
            data_addr = '(simplefs_addr_t) %d' % binindex
        else:
            data_addr = '(simplefs_addr_t) &_data'+sym+'[0]'

        print "Processing %s" % name
        decl = 'fs_file_t %s = {%s,%d,%d,%s,%s};' % (sym,
                                                     next_ptr,
                                                     fs_type,
                                                     length,
                                                     data_addr,
                                                     to_char_array(name))
        decls.append(decl)

        if is_flash_fs:
            binfile.write(data)
            binindex += length
        else:
            decl = 'char %s[] = %s;' % ('_data'+sym,
                                        to_char_array(data,add_null=False))
            decls.append(decl)


    for i in range(len(subdirs)):
        traverse(os.path.join(root,subdirs[i]),
                 next_ptr = get_next_ptr(root,subdirs,i),
                 name = subdirs[i])

if __name__ == "__main__":
    root = sys.argv[1]
    cpath = sys.argv[2]
    binpath = sys.argv[3]
    hpath = sys.argv[4]

    is_flash_fs = False
    if os.path.exists(os.path.join(root,'webserver.conf')):
        print "Found webserver.conf"
        config = ConfigParser.ConfigParser()
        config.read(os.path.join(root,'webserver.conf'))
        try:
            if config.get('Webserver','use_flash') in ('true','True'):
                is_flash_fs = True
        except:
            pass

    if is_flash_fs:
        print "Generating web pages for flash"
        print "*************************************"
        print "YOU HAVE TO REFLASH THE DEVICE FOR THE PROGRAM TO WORK"
        print "*************************************"
    else:
        print "Generating web pages in program image"

    decls = []
    dyn_exprs = {}
    dyn_expr_count = 0;
    binindex = 4
    if is_flash_fs:
        binfile = open(binpath,"wb")
        #Add a signature at the beginning of the image
        binfile.write(chr(0x7e))
        binfile.write(chr(0x51))
        binfile.write(chr(0xeb))
        binfile.write(chr(0x03))

    traverse(root)
    if is_flash_fs:
        binfile.close()
    decls.reverse()
    f = open(cpath,"w")
    includes = ['simplefs.h','stdlib.h','web_server.h']
    for inc in includes:
        f.write('#include "%s"\n\n'%inc)

    f.write('#ifdef __web_server_conf_h_exists__\n')
    f.write('#include "web_server_conf.h"\n')
    f.write('#endif\n\n')

    for d in decls:
        f.write(d+'\n\n')



    f.write('int web_server_dyn_expr(int exp, char *buf, int app_state, int connection_state)\n{\n')
    f.write('  switch (exp) {\n')
    for expr, i in dyn_exprs.iteritems():
        f.write('   case %d: return %s;\n' % (i, expr))
    f.write('  }\n')
    f.write('  return 0;\n')
    f.write('}\n\n')
    f.write('fs_dir_t *root = &%s;\n\n' % get_id(root))
    f.close()

    f = open(hpath,"w")
    f.write('#ifndef __web_server_gen_h__\n')
    f.write('#define __web_server_gen_h__\n')
    f.write('#define WEB_SERVER_IMAGE_SIZE %d\n' % binindex)
    f.write('#endif\n')
    f.close()
