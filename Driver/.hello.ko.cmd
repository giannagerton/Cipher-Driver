cmd_/home/superuser/Driver/hello.ko := ld -r -m elf_x86_64 -z max-page-size=0x200000 -T ./scripts/module-common.lds --build-id  -o /home/superuser/Driver/hello.ko /home/superuser/Driver/hello.o /home/superuser/Driver/hello.mod.o ;  true
