var test1 = new Vue({
    el: '#test1',
    data: {
        isActive:true,
        error: null
    },
    computed: {
        classObject: function(){
            return {
                active :this.isActive && !this.error,
                'text-danger': this.error && error.type === 'fatal'
            }
        }
    }
})

var test2 = new Vue({
    el: '#test2',
    data: {
        activeColor: 'red',
        fontSize: 30
    }
})

var test3 = new Vue({
    el: '#test3',
    data: {
    styleObject: {
        color: 'red',
        fontSize: '20px'
    }
}
})