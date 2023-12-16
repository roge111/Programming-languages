%define NEXT_ELEMENT 0
%macro colon 2
    %ifid %2
        %2: dq NEXT_ELEMENT
        %define NEXT_ELEMENT %2
    %else
        %error "second argument should be id"
    %endif

    %ifstr %1
        db %1, 0
    %else 
        %error "first argument should be string"
    %endif
%endmacro